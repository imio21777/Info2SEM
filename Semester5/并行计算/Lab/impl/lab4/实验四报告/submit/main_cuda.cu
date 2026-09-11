/* 
 * CUDA Optimized Version
 * logDataVSPrior is a function to calculate 
 * the accumulation from ABS of two groups of complex data
 * *************************************************************************/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cuda_runtime.h>

using namespace std;
typedef chrono::high_resolution_clock Clock;

const int m=1638400;	// DO NOT CHANGE!!
const int K=100000;	// DO NOT CHANGE!!

// CUDA error checking macro
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            fprintf(stderr, "CUDA error at %s:%d: %s\n", __FILE__, __LINE__, \
                    cudaGetErrorString(err)); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

// Complex number structure for CUDA
struct Complex {
    double real;
    double imag;
    
    __host__ __device__ Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}
    
    __host__ __device__ Complex operator-(const Complex& other) const {
        return Complex(real - other.real, imag - other.imag);
    }
    
    __host__ __device__ Complex operator*(double scalar) const {
        return Complex(real * scalar, imag * scalar);
    }
};

// Compute norm (squared magnitude) of complex number
__device__ double norm(const Complex& c) {
    return c.real * c.real + c.imag * c.imag;
}

// CUDA kernel for computing partial results
__global__ void computeKernel(
    const Complex* dat, 
    const Complex* pri,
    const double* ctf, 
    const double* sigRcp,
    double* partial_results,
    int num
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx < num) {
        // Compute: dat[idx] - ctf[idx] * pri[idx]
        Complex diff = dat[idx] - (pri[idx] * ctf[idx]);
        
        // Compute norm and multiply by sigRcp
        partial_results[idx] = norm(diff) * sigRcp[idx];
    }
}

// Parallel reduction kernel using shared memory
__global__ void reductionKernel(double* input, double* output, int n) {
    extern __shared__ double sdata[];
    
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x * (blockDim.x * 2) + threadIdx.x;
    
    // Load data to shared memory with grid-stride loop
    double mySum = 0.0;
    if (i < n) mySum += input[i];
    if (i + blockDim.x < n) mySum += input[i + blockDim.x];
    sdata[tid] = mySum;
    __syncthreads();
    
    // Reduction in shared memory
    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }
    
    // Write result for this block to global memory
    if (tid == 0) output[blockIdx.x] = sdata[0];
}

// Host function to perform complete reduction
double deviceReduce(double* d_input, int n) {
    const int blockSize = 256;
    int gridSize = (n + blockSize * 2 - 1) / (blockSize * 2);
    
    double* d_temp;
    CUDA_CHECK(cudaMalloc(&d_temp, gridSize * sizeof(double)));
    
    size_t sharedMemSize = blockSize * sizeof(double);
    reductionKernel<<<gridSize, blockSize, sharedMemSize>>>(d_input, d_temp, n);
    
    // If we still have multiple blocks, reduce again
    while (gridSize > 1) {
        int newGridSize = (gridSize + blockSize * 2 - 1) / (blockSize * 2);
        reductionKernel<<<newGridSize, blockSize, sharedMemSize>>>(d_temp, d_temp, gridSize);
        gridSize = newGridSize;
    }
    
    double result;
    CUDA_CHECK(cudaMemcpy(&result, d_temp, sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_temp));
    
    return result;
}

double logDataVSPrior(
    const Complex* d_dat, 
    const Complex* d_pri, 
    const double* d_ctf, 
    const double* d_sigRcp,
    double* d_partial,
    int num, 
    double disturb0
);

int main ( int argc, char *argv[] )
{ 
    Complex *dat = new Complex[m];
    Complex *pri = new Complex[m];
    double *ctf = new double[m];
    double *sigRcp = new double[m];
    double *disturb = new double[K];
    double dat0, dat1, pri0, pri1, ctf0, sigRcp0;

    /***************************
     * Read data from input.dat
     * *************************/
    ifstream fin;

    fin.open("input.dat");
    if(!fin.is_open())
    {
        cout << "Error opening file input.dat" << endl;
        exit(1);
    }
    int i=0;
    while( !fin.eof() ) 
    {
        fin >> dat0 >> dat1 >> pri0 >> pri1 >> ctf0 >> sigRcp0;
        dat[i] = Complex(dat0, dat1);
        pri[i] = Complex(pri0, pri1);
        ctf[i] = ctf0;
        sigRcp[i] = sigRcp0;
        i++;
        if(i == m) break;
    }
    fin.close();

    fin.open("K.dat");
    if(!fin.is_open())
    {
	cout << "Error opening file K.dat" << endl;
	exit(1);
    }
    i=0;
    while( !fin.eof() )
    {
	fin >> disturb[i];
	i++;
	if(i == K) break;
    }
    fin.close();

    // Allocate device memory
    Complex *d_dat, *d_pri;
    double *d_ctf, *d_sigRcp, *d_partial;
    
    CUDA_CHECK(cudaMalloc(&d_dat, m * sizeof(Complex)));
    CUDA_CHECK(cudaMalloc(&d_pri, m * sizeof(Complex)));
    CUDA_CHECK(cudaMalloc(&d_ctf, m * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_sigRcp, m * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_partial, m * sizeof(double)));
    
    // Copy data to device
    CUDA_CHECK(cudaMemcpy(d_dat, dat, m * sizeof(Complex), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_pri, pri, m * sizeof(Complex), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_ctf, ctf, m * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_sigRcp, sigRcp, m * sizeof(double), cudaMemcpyHostToDevice));

    /***************************
     * main computation is here
     * ************************/
    auto startTime = Clock::now(); 

    ofstream fout;
    fout.open("result.dat");
    if(!fout.is_open())
    {
         cout << "Error opening file for result" << endl;
         exit(1);
    }

    for(unsigned int t = 0; t < K; t++)
    {
        double result = logDataVSPrior(d_dat, d_pri, d_ctf, d_sigRcp, d_partial, m, disturb[t]);
        fout << t+1 << ": " << result << endl;
    }
    fout.close();

    auto endTime = Clock::now(); 

    auto compTime = chrono::duration_cast<chrono::microseconds>(endTime - startTime);
    cout << "Computing time=" << compTime.count() << " microseconds" << endl;

    // Free device memory
    CUDA_CHECK(cudaFree(d_dat));
    CUDA_CHECK(cudaFree(d_pri));
    CUDA_CHECK(cudaFree(d_ctf));
    CUDA_CHECK(cudaFree(d_sigRcp));
    CUDA_CHECK(cudaFree(d_partial));

    delete[] dat;
    delete[] pri;
    delete[] ctf;
    delete[] sigRcp;
    delete[] disturb;
    
    return EXIT_SUCCESS;
}

double logDataVSPrior(
    const Complex* d_dat, 
    const Complex* d_pri, 
    const double* d_ctf, 
    const double* d_sigRcp,
    double* d_partial,
    int num, 
    double disturb0
)
{
    const int blockSize = 256;
    const int gridSize = (num + blockSize - 1) / blockSize;
    
    // Launch kernel to compute partial results
    computeKernel<<<gridSize, blockSize>>>(d_dat, d_pri, d_ctf, d_sigRcp, d_partial, num);
    CUDA_CHECK(cudaGetLastError());
    
    // Perform reduction to get final sum
    double result = deviceReduce(d_partial, num);
    
    return result * disturb0; // DO NOT CHANGE!!
}
