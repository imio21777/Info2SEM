#include<iostream>
#include<sys/time.h>
#include<stdlib.h>
#include<stdio.h>
#include<cuda.h>

#define N 500
#define ITERATIONS 10
#define DIM_THREAD_BLOCK_X 32
#define DIM_THREAD_BLOCK_Y 8
using namespace std;

__global__ void sgemm(float *A, float *B, float *C, int n, float a, float b) {
  int j = blockIdx.x * blockDim.x + threadIdx.x;
  int i = blockIdx.y * blockDim.y + threadIdx.y;
  if( (i<n) && (j<n) ){
    float tmp = b*C[i*n+j];
    for(int k=0; k<n; k++){
      tmp += a*A[i*n+k]*B[k*n+j];
    }
    C[i*n+j]=tmp;
  }
}

void compare(float* res1, float* res2, int n){
  int fail=0;
  for(int i=0; i<n; i++){
    float a,b;
    if(res1[i]<0)
      a=res1[i]*(-1);
    else 
      a=res1[i];
    if(res2[i]<0)
      b=res2[i]*(-1);
    else 
      b=res2[i];
    if((a<0.01)&&(b<0.01)){
      continue;
    }
    if(i<10)
      printf("i=%d %lf %lf\n",i,a,b);
    float diff=(a-b)/(a+0.000001);
    if(diff<0)
      diff=diff*(-1);
    if(diff>0.0005)
      fail++;
  }
  printf("Number of errors: %d\n", fail);
}

double timestamp(){
  struct timeval tv;
  gettimeofday (&tv, 0);
  return tv.tv_sec + 1e-6*tv.tv_usec;
}

int main(){
  float A[N*N], B[N*N], C_cpu[N*N], C_gpu_final[N*N];
  //float A[N][N], B[N][N], C_cpu[N][N], C_gpu_final[N][N];
  float a=0.5, b=0.3;
  for(int i=0; i<N; i++){
    for(int j=0; j<N; j++){
      A[i*N+j]=(float)rand()/(float)(RAND_MAX/a);
      B[i*N+j]=(float)rand()/(float)(RAND_MAX/a);
      C_cpu[i*N+j]=0;
      C_gpu_final[i*N+j]=0;
    }
  }

  for(int j=0; j<N; j++){
    for(int i=0; i<N; i++){
      C_cpu[i*N+j]+=b*C_cpu[i*N+j];
      for(int k=0; k<N; k++){
        C_cpu[i*N+j] += a*A[i*N+k]*B[k*N+j];
      }
    }
  }

  float *A_gpu;
  float *B_gpu;
  float *C_gpu;
  cudaMalloc((void **)&A_gpu, sizeof(float)*N*N);
  cudaMalloc((void **)&B_gpu, sizeof(float)*N*N);
  cudaMalloc((void **)&C_gpu, sizeof(float)*N*N);
  cudaMemcpy(A_gpu, A, sizeof(float)*N*N, cudaMemcpyHostToDevice);
  cudaMemcpy(B_gpu, B, sizeof(float)*N*N, cudaMemcpyHostToDevice);
  cudaMemcpy(C_gpu, C_gpu_final, sizeof(float)*N*N, cudaMemcpyHostToDevice);

  dim3 block(DIM_THREAD_BLOCK_X, DIM_THREAD_BLOCK_Y);
  dim3 grid((size_t)ceil( ((float)N) / ((float)block.x) ), (size_t)ceil( ((float)N) / ((float)block.y)) );

  sgemm<<<grid,block>>>(A_gpu, B_gpu, C_gpu, N, a, b);
  cudaThreadSynchronize();
  cudaMemcpy(C_gpu_final, C_gpu, sizeof(float)*N*N, cudaMemcpyDeviceToHost);
  compare(C_cpu, C_gpu_final, N*N);

  double time1=timestamp();
  for(int numOfTimes=0; numOfTimes<ITERATIONS; numOfTimes++){

    sgemm<<<grid,block>>>(A_gpu, B_gpu, C_gpu, N, a, b);

  }
  cudaThreadSynchronize();
  double time2=timestamp();

  double time = (time2-time1)/ITERATIONS;
  double flops = 2*N*N*N;
  double gflopsPerSecond = flops/(1000000000)/time;
  double GB = (double)(N)*N*4/1000000000;
  double GBpS = (double)(N)*N*4/1000000000/time;
  printf("GFLOPS/s=%lf\n",gflopsPerSecond );
  printf("GB/s=%lf\n",GBpS);
  printf("GFLOPS=%lf\n",flops/(1000000000));
  printf("GB=%lf\n",GB);
  printf("time(s)=%lf\n",time);

  
  cudaFree(A_gpu);
  cudaFree(B_gpu);
  cudaFree(C_gpu);
  return 0;
}
