#include <iostream>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include </opt/homebrew/Cellar/libomp/21.1.0/include/omp.h> // 引入OpenMP头文件
#include <vector>

using namespace std;

double timestamp()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + 1e-6 * tv.tv_usec;
}

// OpenMP优化的矩阵乘法函数 C = aAB + bC
// 使用一维数组模拟二维矩阵
void gemm_optimized_openmp(int N, float a, float b, const float *A, const float *B, float *C)
{
  // #pragma omp parallel for: 指示编译器将接下来的for循环并行化
  // schedule(static): 静态调度，将迭代任务平均分配给线程，适用于任务量均匀的场景

#pragma omp parallel for schedule(static)
  for (int i = 0; i < N; i++)
  {
    // 先计算 b*C 的部分
    for (int j = 0; j < N; j++)
    {
      C[i * N + j] *= b;
    }

    // 采用 i-k-j 循环顺序以优化缓存
    for (int k = 0; k < N; k++)
    {
      // 将 a * A[i*N + k] 提取到外层，减少重复计算
      const float a_ik = a * A[i * N + k];
      for (int j = 0; j < N; j++)
      {
        // calc a_ik * b_kj
        C[i * N + j] += a_ik * B[k * N + j];
      }
    }
  }
}

// 辅助函数：初始化矩阵
void initialize_matrices(int N, float *A, float *B, float *C)
{
#pragma omp parallel for
  float a = 0.5, b = 0.3;
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      A[i][j] = (float)rand() / (float)(RAND_MAX / a);
      B[i][j] = (float)rand() / (float)(RAND_MAX / a);
      C[i][j] = 0;
    }
  }
}

int main()
{
  // 定义不同规模的矩阵进行测试
  vector<int> sizes = {512, 800, 1024, 1536, 2048};
  const int ITERATIONS = 5; // 迭代次数，取平均值
  const float a = 0.5f, b = 0.3f;

  cout << "======================================================================" << endl;
  cout << "  Matrix Size |   Mem Usage (MB) |   Time (s) |   GFLOPS/s" << endl;
  cout << "----------------------------------------------------------------------" << endl;

  for (int N : sizes)
  {
    // 1. 在堆上动态分配内存
    float *A = new float[N * N];
    float *B = new float[N * N];
    float *C = new float[N * N];

    if (!A || !B || !C)
    {
      cerr << "Failed to allocate memory for N=" << N << endl;
      continue;
    }

    // 计算内存占用 (3个矩阵)
    double mem_usage_mb = (double)3 * N * N * sizeof(float) / (1024.0 * 1024.0);

    // 2. 初始化矩阵
    initialize_matrices(N, A, B, C);

    // 3. 预热，确保CPU频率稳定，并减少首次运行的缓存未命中影响
    gemm_optimized_openmp(N, a, b, A, B, C);

    // 4. 正式测试并计时
    double start_time = timestamp();
    for (int i = 0; i < ITERATIONS; ++i)
    {
      // 注意：每次迭代C矩阵的值都会改变，严格来说应重置C
      // 但对于性能测试，重复计算影响不大，我们关注的是计算本身的速度
      gemm_optimized_openmp(N, a, b, A, B, C);
    }
    double end_time = timestamp();

    // 5. 计算性能
    double avg_time = (end_time - start_time) / ITERATIONS;

    // 计算GFLOPS/秒
    // 对于 C = aAB + bC:
    // C[i][j] *= b: 1次乘法 (N*N次)
    // a_ik = a * A[i*N+k]: 1次乘法 (N*N次)
    // C[i*N+j] += a_ik * B[k*N+j]: 1次乘法, 1次加法 (N*N*N次)
    // 总FLOPs = N*N (for bC) + N*N (for a*A) + 2*N*N*N (for AB)
    // 简化为 2*N^3 + 2*N^2
    double flops = (2.0 * N * N * N + 2.0 * N * N);
    double gflops_per_second = flops / (1e9 * avg_time);

    printf(" %12d | %16.2f | %10.4f | %12.2f\n", N, mem_usage_mb, avg_time, gflops_per_second);

    // 6. 释放内存
    delete[] A;
    delete[] B;
    delete[] C;
  }

  cout << "======================================================================" << endl;

  return 0;
}