#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <gmp.h>
#include "cgbn/cgbn.h"

// Adjust BITS to match n^2 (e.g. if n is 2048 bits, n^2 is 4096 bits)
#define BITS 4096
#define TPI 32  // Threads Per Instance
#define LIMBS (BITS / 32)

// Kernel configuration
#define IPB 4   // Instances Per Block
#define TPB (IPB * TPI)

typedef cgbn_context_t<TPI>         context_t;
typedef cgbn_env_t<context_t, BITS> env_t;
typedef typename env_t::cgbn_t      bn_t;
typedef typename env_t::cgbn_wide_t bn_wide_t;

// Error checking macro
#define CUDA_CHECK(call) { \
    cudaError_t err = call; \
    if (err != cudaSuccess) { \
        printf("CUDA error at %s:%d: %s\n", __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
}

#define CGBN_CHECK(report) { \
    cgbn_error_report_t *h_report = (cgbn_error_report_t*)malloc(sizeof(cgbn_error_report_t)); \
    cudaMemcpy(h_report, report, sizeof(cgbn_error_report_t), cudaMemcpyDeviceToHost); \
    if (cgbn_error_report_check(h_report)) { \
        printf("CGBN error at %s:%d\n", __FILE__, __LINE__); \
        free(h_report); \
        exit(1); \
    } \
    free(h_report); \
}

// Encryption Kernel
// c = (1 + m*n) * r^n mod n^2
// Encryption Kernel
// c = (1 + m*n) * r^n mod n^2
__global__ void paillier_encrypt(cgbn_error_report_t *report, uint32_t *c_out, uint32_t *m_in, uint32_t *r_in, uint32_t *n_in, uint32_t *nsquare_in, int count) {
    int instance = (blockIdx.x * blockDim.x + threadIdx.x) / TPI;
    if (instance >= count) return;

    context_t context(cgbn_report_monitor, report, instance);
    env_t          env(context.env<env_t>());
    
    bn_t m, r, n, n2, c, term1, term2;
    
    // 定义一个宽变量，用于存储 2*BITS (8192 bits) 的结果
    bn_wide_t wide_result; 
    
    // Load inputs
    cgbn_load(env, m, (cgbn_mem_t<BITS>*)(m_in + instance * LIMBS));
    cgbn_load(env, r, (cgbn_mem_t<BITS>*)(r_in + instance * LIMBS));
    cgbn_load(env, n, (cgbn_mem_t<BITS>*)n_in); 
    cgbn_load(env, n2, (cgbn_mem_t<BITS>*)nsquare_in);

    // term1 = (1 + m*n) mod n^2
    cgbn_mul(env, term1, m, n);
    cgbn_add_ui32(env, term1, term1, 1);
    cgbn_rem(env, term1, term1, n2); 

    // term2 = r^n mod n^2
    cgbn_modular_power(env, term2, r, n, n2);

    // c = term1 * term2 mod n^2
    // -----------------------------------------------------------
    // 修正点：使用 wide_result (8192位) 承接乘法结果
    // 函数签名: void mul_wide(cgbn_wide_t &r, const cgbn_t &a, const cgbn_t &b);
    // -----------------------------------------------------------
    env.mul_wide(wide_result, term1, term2);

    // -----------------------------------------------------------
    // 修正点：从 wide_result 中取模
    // 函数签名: void rem_wide(cgbn_t &r, const cgbn_wide_t &a, const cgbn_t &m);
    // -----------------------------------------------------------
    env.rem_wide(c, wide_result, n2);

    // Store result
    cgbn_store(env, (cgbn_mem_t<BITS>*)(c_out + instance * LIMBS), c);
}

// Decryption Kernel
// m = L(c^lambda mod n^2) * mu mod n
// L(x) = (x-1)/n
__global__ void paillier_decrypt(cgbn_error_report_t *report, uint32_t *m_out, uint32_t *c_in, uint32_t *lam_in, uint32_t *mu_in, uint32_t *n_in, uint32_t *nsquare_in, int count) {
    int instance = (blockIdx.x * blockDim.x + threadIdx.x) / TPI;
    if (instance >= count) return;

    context_t context(cgbn_report_monitor, report, instance);
    env_t          env(context.env<env_t>());
    
    bn_t c, lam, mu, n, n2, m, temp;
    
    cgbn_load(env, c, (cgbn_mem_t<BITS>*)(c_in + instance * LIMBS));
    cgbn_load(env, lam, (cgbn_mem_t<BITS>*)lam_in);
    cgbn_load(env, mu, (cgbn_mem_t<BITS>*)mu_in);
    cgbn_load(env, n, (cgbn_mem_t<BITS>*)n_in);
    cgbn_load(env, n2, (cgbn_mem_t<BITS>*)nsquare_in);

    // temp = c^lambda mod n^2
    cgbn_modular_power(env, temp, c, lam, n2);

    // L(temp) = (temp - 1) / n
    cgbn_sub_ui32(env, temp, temp, 1);
    cgbn_div(env, temp, temp, n); // Integer division

    // m = temp * mu mod n
    // temp (~2048 bits) * mu (~2048 bits) 结果约为 4096 bits，刚好放入 BITS
    // 因此这里用普通 mul 配合 rem 是安全的，只要 product 不超过 2^4096-1
    cgbn_mul(env, m, temp, mu);
    cgbn_rem(env, m, m, n); 

    cgbn_store(env, (cgbn_mem_t<BITS>*)(m_out + instance * LIMBS), m);
}

// Helper to generate random numbers (using GMP)
// 修正：增加 max_n 参数，确保 r < n
void generate_random(uint32_t *data, int count, int bits, gmp_randstate_t state, mpz_t limit) {
    mpz_t r;
    mpz_init(r);
    for (int i = 0; i < count; i++) {
        if (limit != NULL) {
            mpz_urandomm(r, state, limit); // Generate 0 <= r < limit
        } else {
            mpz_urandomb(r, state, bits);  // Generate 0 <= r < 2^bits
        }
        
        size_t countp;
        memset(data + i * LIMBS, 0, LIMBS * 4);
        mpz_export(data + i * LIMBS, &countp, -1, 4, 0, 0, r);
    }
    mpz_clear(r);
}

int main(int argc, char **argv) {
    int count = 1000; 
    if (argc > 1) count = atoi(argv[1]);
    
    printf("Running Paillier GPU Benchmark with %d instances...\n", count);

    uint32_t *h_m = (uint32_t*)malloc(count * LIMBS * 4);
    uint32_t *h_r = (uint32_t*)malloc(count * LIMBS * 4);
    uint32_t *h_c = (uint32_t*)malloc(count * LIMBS * 4);
    uint32_t *h_dec = (uint32_t*)malloc(count * LIMBS * 4);
    
    uint32_t *h_n = (uint32_t*)malloc(LIMBS * 4);
    uint32_t *h_nsquare = (uint32_t*)malloc(LIMBS * 4);
    uint32_t *h_lam = (uint32_t*)malloc(LIMBS * 4);
    uint32_t *h_mu = (uint32_t*)malloc(LIMBS * 4);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    
    mpz_t p, q, n, n2, lam, mu, pm1, qm1;
    mpz_inits(p, q, n, n2, lam, mu, pm1, qm1, NULL);

    int prime_bits = 1024;
    mpz_urandomb(p, state, prime_bits); mpz_nextprime(p, p);
    mpz_urandomb(q, state, prime_bits); mpz_nextprime(q, q);
    
    mpz_mul(n, p, q);
    mpz_mul(n2, n, n);
    
    mpz_sub_ui(pm1, p, 1);
    mpz_sub_ui(qm1, q, 1);
    mpz_lcm(lam, pm1, qm1); 
    
    mpz_invert(mu, lam, n);

    size_t countp;
    memset(h_n, 0, LIMBS*4); mpz_export(h_n, &countp, -1, 4, 0, 0, n);
    memset(h_nsquare, 0, LIMBS*4); mpz_export(h_nsquare, &countp, -1, 4, 0, 0, n2);
    memset(h_lam, 0, LIMBS*4); mpz_export(h_lam, &countp, -1, 4, 0, 0, lam);
    memset(h_mu, 0, LIMBS*4); mpz_export(h_mu, &countp, -1, 4, 0, 0, mu);
    
    // Generate data
    // m 必须 < n, r 必须 < n
    generate_random(h_m, count, 1024, state, NULL); // m 只有1024位，肯定小于 n (2048位)
    generate_random(h_r, count, 2048, state, n);    // r < n

    // Device memory
    uint32_t *d_m, *d_r, *d_c, *d_n, *d_nsquare, *d_lam, *d_mu, *d_dec;
    cgbn_error_report_t *d_report;

    CUDA_CHECK(cudaMalloc(&d_m, count * LIMBS * 4));
    CUDA_CHECK(cudaMalloc(&d_r, count * LIMBS * 4));
    CUDA_CHECK(cudaMalloc(&d_c, count * LIMBS * 4));
    CUDA_CHECK(cudaMalloc(&d_dec, count * LIMBS * 4));
    CUDA_CHECK(cudaMalloc(&d_n, LIMBS * 4));
    CUDA_CHECK(cudaMalloc(&d_nsquare, LIMBS * 4));
    CUDA_CHECK(cudaMalloc(&d_lam, LIMBS * 4));
    CUDA_CHECK(cudaMalloc(&d_mu, LIMBS * 4));
    CUDA_CHECK(cudaMalloc(&d_report, sizeof(cgbn_error_report_t)));

    CUDA_CHECK(cudaMemcpy(d_m, h_m, count * LIMBS * 4, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_r, h_r, count * LIMBS * 4, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_n, h_n, LIMBS * 4, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_nsquare, h_nsquare, LIMBS * 4, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_lam, h_lam, LIMBS * 4, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_mu, h_mu, LIMBS * 4, cudaMemcpyHostToDevice));
    
    CUDA_CHECK(cudaMemset(d_report, 0, sizeof(cgbn_error_report_t)));

    int threadsPerBlock = TPB;
    int blocks = (count + IPB - 1) / IPB;

    printf("Launching Encryption Kernel...\n");
    cudaEvent_t start, stop;
    cudaEventCreate(&start); cudaEventCreate(&stop);
    
    cudaEventRecord(start);
    paillier_encrypt<<<blocks, threadsPerBlock>>>(d_report, d_c, d_m, d_r, d_n, d_nsquare, count);
    cudaEventRecord(stop);
    
    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    
    CGBN_CHECK(d_report);
    printf("Encryption Time: %.2f ms (Avg: %.4f ms)\n", milliseconds, milliseconds/count);

    // Decryption
    printf("Launching Decryption Kernel...\n");
    cudaEventRecord(start);
    paillier_decrypt<<<blocks, threadsPerBlock>>>(d_report, d_dec, d_c, d_lam, d_mu, d_n, d_nsquare, count);
    cudaEventRecord(stop);
    
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&milliseconds, start, stop);
    
    CGBN_CHECK(d_report);
    printf("Decryption Time: %.2f ms (Avg: %.4f ms)\n", milliseconds, milliseconds/count);

    CUDA_CHECK(cudaMemcpy(h_dec, d_dec, count * LIMBS * 4, cudaMemcpyDeviceToHost));
    
    int errors = 0;
    for (int i = 0; i < count; i++) {
        if (memcmp(h_m + i * LIMBS, h_dec + i * LIMBS, LIMBS * 4) != 0) {
            errors++;
            if (errors < 5) printf("Mismatch at %d\n", i);
        }
    }
    
    printf("Verification complete. Errors: %d\n", errors);

    cudaFree(d_m); cudaFree(d_r); cudaFree(d_c); cudaFree(d_dec);
    cudaFree(d_n); cudaFree(d_nsquare); cudaFree(d_lam); cudaFree(d_mu); cudaFree(d_report);
    free(h_m); free(h_r); free(h_c); free(h_dec);
    free(h_n); free(h_nsquare); free(h_lam); free(h_mu);

    return 0;
}