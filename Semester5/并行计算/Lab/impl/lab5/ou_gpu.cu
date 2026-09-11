#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <gmp.h>
#include "cgbn/cgbn.h"

// OU modulus n = p^2 * q.
// If primes are 1024 bit, p^2 is 2048, n is 3072.
// Use 4096 bit context.
#define BITS 4096
#define TPI 32
#define LIMBS (BITS / 32)
#define IPB 4
#define TPB (IPB * TPI)

typedef cgbn_context_t<TPI> context_t;
typedef cgbn_env_t<context_t, BITS> env_t;
typedef typename env_t::cgbn_t bn_t;
typedef typename env_t::cgbn_wide_t bn_wide_t;

#define CUDA_CHECK(call)                                                       \
  {                                                                            \
    cudaError_t err = call;                                                    \
    if (err != cudaSuccess) {                                                  \
      printf("CUDA error at %s:%d: %s\n", __FILE__, __LINE__, cudaGetErrorString(err)); \
      exit(1);                                                                 \
    }                                                                          \
  }

#define CGBN_CHECK(report)                                                     \
  {                                                                            \
    cgbn_error_report_t *h_report = (cgbn_error_report_t*)malloc(sizeof(cgbn_error_report_t)); \
    cudaMemcpy(h_report, report, sizeof(cgbn_error_report_t), cudaMemcpyDeviceToHost); \
    if (cgbn_error_report_check(h_report)) {                                   \
      printf("CGBN error at %s:%d\n", __FILE__, __LINE__);                     \
      printf("Instance: %d, Error Code: %d\n", h_report->_instance, h_report->_error); \
      free(h_report);                                                          \
      exit(1);                                                                 \
    }                                                                          \
    free(h_report);                                                            \
  }

// Encrypt: c = g^m * h^r mod n
__global__ void ou_encrypt_kernel(cgbn_error_report_t *report, uint32_t *c_out,
                                  uint32_t *m_in, uint32_t *r_in,
                                  uint32_t *g_in, uint32_t *h_in,
                                  uint32_t *n_in, int count) {
  int instance = (blockIdx.x * blockDim.x + threadIdx.x) / TPI;
  if (instance >= count)
    return;
  context_t context(cgbn_report_monitor, report, instance);
  env_t env(context.env<env_t>());

  bn_t m, r, g, h, n, t1, t2;
  bn_wide_t wide_mul;

  cgbn_load(env, m, (cgbn_mem_t<BITS>*)(m_in + instance * LIMBS));
  cgbn_load(env, r, (cgbn_mem_t<BITS>*)(r_in + instance * LIMBS));
  cgbn_load(env, g, (cgbn_mem_t<BITS>*)g_in); 
  cgbn_load(env, h, (cgbn_mem_t<BITS>*)h_in);
  cgbn_load(env, n, (cgbn_mem_t<BITS>*)n_in);

  // t1 = g^m mod n
  cgbn_modular_power(env, t1, g, m, n);
  // t2 = h^r mod n
  cgbn_modular_power(env, t2, h, r, n);

  // c = t1 * t2 mod n
  env.mul_wide(wide_mul, t1, t2);
  env.rem_wide(t1, wide_mul, n);

  cgbn_store(env, (cgbn_mem_t<BITS>*)(c_out + instance * LIMBS), t1);
}

// Decrypt: m = L(c^(p-1) mod p^2) * L(g^(p-1) mod p^2)^-1 mod p
__global__ void ou_decrypt_kernel(cgbn_error_report_t *report, uint32_t *m_out,
                                  uint32_t *c_in, uint32_t *p_in,
                                  uint32_t *p_sq_in, uint32_t *pm1_in,
                                  uint32_t *inv_L_gp_in, int count) {
  int instance = (blockIdx.x * blockDim.x + threadIdx.x) / TPI;
  if (instance >= count)
    return;
  context_t context(cgbn_report_monitor, report, instance);
  env_t env(context.env<env_t>());

  bn_t c, p, p_sq, pm1, inv_L_gp, cp, temp;

  cgbn_load(env, c, (cgbn_mem_t<BITS>*)(c_in + instance * LIMBS));
  cgbn_load(env, p, (cgbn_mem_t<BITS>*)p_in);
  cgbn_load(env, p_sq, (cgbn_mem_t<BITS>*)p_sq_in);
  cgbn_load(env, pm1, (cgbn_mem_t<BITS>*)pm1_in);
  cgbn_load(env, inv_L_gp, (cgbn_mem_t<BITS>*)inv_L_gp_in);

  // [修复2] c 可能大于 p^2 (因为 c < n, n = p^2*q)，必须先取模
  cgbn_rem(env, temp, c, p_sq);

  // cp = temp^(p-1) mod p^2
  cgbn_modular_power(env, cp, temp, pm1, p_sq);

  // L(cp) = (cp - 1) / p
  env.sub_ui32(cp, cp, 1);
  env.div(cp, cp, p); 

  // m = L(cp) * inv_L_gp mod p
  env.mul(temp, cp, inv_L_gp);
  env.rem(temp, temp, p);

  cgbn_store(env, (cgbn_mem_t<BITS>*)(m_out + instance * LIMBS), temp);
}

void to_device(uint32_t *h_ptr, uint32_t **d_ptr, int size) {
  CUDA_CHECK(cudaMalloc(d_ptr, size));
  CUDA_CHECK(cudaMemcpy(*d_ptr, h_ptr, size, cudaMemcpyHostToDevice));
}

void generate_random(uint32_t *data, int count, int bits, gmp_randstate_t state,
                     mpz_t limit) {
  mpz_t r;
  mpz_init(r);
  for (int i = 0; i < count; i++) {
    if (limit)
      mpz_urandomm(r, state, limit);
    else
      mpz_urandomb(r, state, bits);
    size_t countp;
    memset(data + i * LIMBS, 0, LIMBS * 4);
    mpz_export(data + i * LIMBS, &countp, -1, 4, 0, 0, r);
  }
  mpz_clear(r);
}

void export_mpz(uint32_t *dest, mpz_t src) {
  size_t countp;
  memset(dest, 0, LIMBS * 4);
  mpz_export(dest, &countp, -1, 4, 0, 0, src);
}

int main(int argc, char **argv) {
  int count = 1000;
  if (argc > 1)
    count = atoi(argv[1]);

  printf("Running OU Benchmark with %d instances...\n", count);

  gmp_randstate_t state;
  gmp_randinit_default(state);

  // Setup keys
  mpz_t p, q, n, p_sq, pm1, g, h, temp, gp, l_gp, inv_l_gp;
  mpz_t g_p_val, gcd_val; 
  mpz_inits(p, q, n, p_sq, pm1, g, h, temp, gp, l_gp, inv_l_gp, g_p_val, gcd_val, NULL);

  mpz_urandomb(p, state, 1024);
  mpz_nextprime(p, p);
  mpz_urandomb(q, state, 1024);
  mpz_nextprime(q, q);

  mpz_mul(p_sq, p, p);
  mpz_mul(n, p_sq, q);
  mpz_sub_ui(pm1, p, 1);

  // g generation
  while (1) {
    mpz_urandomm(g, state, n); 
    if (mpz_cmp_ui(g, 1) <= 0) continue; 

    // 1. gcd(g, n) 必须为 1
    mpz_gcd(gcd_val, g, n);
    if (mpz_cmp_ui(gcd_val, 1) != 0) continue; 

    // 2. g^(p-1) mod p^2
    mpz_powm(g_p_val, g, pm1, p_sq);

    // 3. L(g^(p-1)) = (g_p_val - 1) / p
    mpz_sub_ui(l_gp, g_p_val, 1);
    if (mpz_divisible_p(l_gp, p) == 0) continue; 
    
    mpz_divexact(l_gp, l_gp, p); 

    // 4. L(g^(p-1)) 必须与 p 互质
    mpz_gcd(gcd_val, l_gp, p);
    if (mpz_cmp_ui(gcd_val, 1) != 0) continue; 

    // [修复1] 在清空 g_p_val 之前保存到 gp
    mpz_set(gp, g_p_val);
    break; 
  }
  
  // 现在可以安全清空临时变量了
  mpz_clears(g_p_val, gcd_val, NULL);

  // h = g^n mod n
  mpz_powm(h, g, n, n);

  // Precompute inv_L_gp
  // gp 已经持有正确的值
  mpz_sub_ui(l_gp, gp, 1);
  mpz_divexact(l_gp, l_gp, p); // L(gp)
  mpz_invert(inv_l_gp, l_gp, p); // 计算逆元

  // Prepare Host Data
  uint32_t *h_m = (uint32_t *)malloc(count * LIMBS * 4);
  uint32_t *h_r = (uint32_t *)malloc(count * LIMBS * 4);
  uint32_t *h_c = (uint32_t *)malloc(count * LIMBS * 4);
  uint32_t *h_dec = (uint32_t *)malloc(count * LIMBS * 4);

  uint32_t *h_g = (uint32_t *)malloc(LIMBS * 4);
  uint32_t *h_h = (uint32_t *)malloc(LIMBS * 4);
  uint32_t *h_n = (uint32_t *)malloc(LIMBS * 4);
  uint32_t *h_p = (uint32_t *)malloc(LIMBS * 4);
  uint32_t *h_p_sq = (uint32_t *)malloc(LIMBS * 4);
  uint32_t *h_pm1 = (uint32_t *)malloc(LIMBS * 4);
  uint32_t *h_inv = (uint32_t *)malloc(LIMBS * 4);

  export_mpz(h_g, g);
  export_mpz(h_h, h);
  export_mpz(h_n, n);
  export_mpz(h_p, p);
  export_mpz(h_p_sq, p_sq);
  export_mpz(h_pm1, pm1);
  export_mpz(h_inv, inv_l_gp);

  generate_random(h_m, count, 0, state, p); // m < p
  generate_random(h_r, count, 0, state, n); // r < n

  // Device Data
  uint32_t *d_m, *d_r, *d_c, *d_dec;
  uint32_t *d_g, *d_h, *d_n, *d_p, *d_p_sq, *d_pm1, *d_inv;
  cgbn_error_report_t *d_report;

  to_device(h_m, &d_m, count * LIMBS * 4);
  to_device(h_r, &d_r, count * LIMBS * 4);
  CUDA_CHECK(cudaMalloc(&d_c, count * LIMBS * 4));
  CUDA_CHECK(cudaMalloc(&d_dec, count * LIMBS * 4));

  to_device(h_g, &d_g, LIMBS * 4);
  to_device(h_h, &d_h, LIMBS * 4);
  to_device(h_n, &d_n, LIMBS * 4);
  to_device(h_p, &d_p, LIMBS * 4);
  to_device(h_p_sq, &d_p_sq, LIMBS * 4);
  to_device(h_pm1, &d_pm1, LIMBS * 4);
  to_device(h_inv, &d_inv, LIMBS * 4);

  CUDA_CHECK(cudaMalloc(&d_report, sizeof(cgbn_error_report_t)));
  CUDA_CHECK(cudaMemset(d_report, 0, sizeof(cgbn_error_report_t)));

  int threadsPerBlock = TPB;
  int blocks = (count + IPB - 1) / IPB;

  printf("Launching OU Encrypt...\n");
  cudaEvent_t start, stop;
  cudaEventCreate(&start); cudaEventCreate(&stop);
  cudaEventRecord(start);
  
  ou_encrypt_kernel<<<blocks, threadsPerBlock>>>(d_report, d_c, d_m, d_r, d_g,
                                                 d_h, d_n, count);
  
  cudaEventRecord(stop);
  cudaEventSynchronize(stop);
  CGBN_CHECK(d_report);
  
  float ms = 0;
  cudaEventElapsedTime(&ms, start, stop);
  printf("Encryption Time: %.2f ms (Avg: %.4f ms)\n", ms, ms/count);

  printf("Launching OU Decrypt...\n");
  cudaEventRecord(start);
  
  ou_decrypt_kernel<<<blocks, threadsPerBlock>>>(d_report, d_dec, d_c, d_p,
                                                 d_p_sq, d_pm1, d_inv, count);
  cudaEventRecord(stop);
  cudaEventSynchronize(stop);
  CGBN_CHECK(d_report);

  cudaEventElapsedTime(&ms, start, stop);
  printf("Decryption Time: %.2f ms (Avg: %.4f ms)\n", ms, ms/count);

  CUDA_CHECK(
      cudaMemcpy(h_dec, d_dec, count * LIMBS * 4, cudaMemcpyDeviceToHost));

  int errors = 0;
  for (int i = 0; i < count; i++) {
    if (memcmp(h_m + i * LIMBS, h_dec + i * LIMBS, LIMBS * 4) != 0) {
      errors++;
      if (errors < 5) printf("Mismatch at %d\n", i);
    }
  }
  printf("Verification complete. Errors: %d\n", errors);

  cudaFree(d_m); cudaFree(d_r); cudaFree(d_c); cudaFree(d_dec);
  cudaFree(d_g); cudaFree(d_h); cudaFree(d_n); cudaFree(d_p); cudaFree(d_p_sq); cudaFree(d_pm1); cudaFree(d_inv);
  cudaFree(d_report);
  free(h_m); free(h_r); free(h_c); free(h_dec);
  free(h_g); free(h_h); free(h_n); free(h_p); free(h_p_sq); free(h_pm1); free(h_inv);
  mpz_clears(p, q, n, p_sq, pm1, g, h, temp, gp, l_gp, inv_l_gp, NULL);
  gmp_randclear(state);

  return 0;
}