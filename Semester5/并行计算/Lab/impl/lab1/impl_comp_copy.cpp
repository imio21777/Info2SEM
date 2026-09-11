#include <iostream>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include </opt/homebrew/Cellar/libomp/21.1.0/include/omp.h>
#include <cstdio>

#define N 800
#define ITERATIONS 10
using namespace std;

bool verify_results(int n, float **C_serial, float **C_parallel)
{
  const float epsilon = 1e-5; // 允许的误差阈值
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      if (std::fabs(C_serial[i][j] - C_parallel[i][j]) > epsilon)
      {
        fprintf(stderr, "Verification FAILED at [%d][%d]! Serial: %f, OMP: %f\n",
                i, j, C_serial[i][j], C_parallel[i][j]);
        return false;
      }
    }
  }
  std::cout << "Verification PASSED!" << std::endl;
  return true;
}

float **allocate_matrix(int n)
{
  // 1. allocate space
  float *data = (float *)malloc(n * n * sizeof(float));
  if (data == nullptr)
  {
    std::cerr << "Error: Failed to allocate memory for matrix data." << std::endl;
    return nullptr;
  }

  // 2. allocate pointer array to store the start addr of each line
  float **matrix = (float **)malloc(n * sizeof(float *));
  if (matrix == nullptr)
  {
    std::cerr << "Error: Failed to allocate memory for matrix pointers." << std::endl;
    free(data);
    return nullptr;
  }

  // 3. change where ptr points to start
  for (int i = 0; i < n; ++i)
  {
    matrix[i] = data + i * n;
  }

  return matrix;
}

void free_matrix(float **matrix)
{
  if (matrix != nullptr)
  {
    // free start
    free(matrix[0]);
    // free ptr array
    free(matrix);
  }
}

// initialize A、B with float in 0~0.5
// initialize C with 0
void initialize_matrices(int n, float **A, float **B, float **C)
{
  float a = 0.5f;
  float b = 0.3f;

  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      A[i][j] = (float)rand() / (float)(RAND_MAX / a);
      B[i][j] = (float)rand() / (float)(RAND_MAX / a);
      C[i][j] = 0;
    }
  }

  return;
}

double timestamp()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + 1e-6 * tv.tv_usec;
}

//  calc C = a*A*B + b*C in serial
void calc_serial(int n, float a, float b, float **A, float **B, float **C)
{
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      float tmp = 0.0f;
      for (int k = 0; k < n; ++k)
      {
        tmp += A[i][k] * B[k][j];
      }
      C[i][j] = a * tmp + b * C[i][j];
    }
  }
}

// calc C = a*A*B + b*C in parallel
void calc_parallel(int n, float a, float b, float **A, float **B, float **C)
{
#pragma omp parallel for schedule(static)
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      float tmp = 0.0f;
      for (int k = 0; k < n; ++k)
      {
        tmp += A[i][k] * B[k][j];
      }
      C[i][j] = a * tmp + b * C[i][j];
    }
  }
}

void yourFunction(float a, float b, float A[N][N], float B[N][N], float C[N][N])
{
  calc_parallel(N, a, b, (float **)A, (float **)B, (float **)C);
  // #pragma omp parallel for schedule(static)
  //   for (int i = 0; i < N; ++i)
  //   {
  //     for (int j = 0; j < N; ++j)
  //     {
  //       float tmp = 0.0f;
  //       for (int k = 0; k < N; ++k)
  //       {
  //         tmp += A[i][k] * B[k][j];
  //       }
  //       C[i][j] = a * tmp + b * C[i][j];
  //     }
  //   }
}

void copy_matrix(int n, float **dest, float **src)
{
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      dest[i][j] = src[i][j];
    }
  }
}

int main()
{
  float a = 0.5, b = 0.3;

  int matrix_sizes[] = {256, 512, 1024, 2048};
  int num_sizes = sizeof(matrix_sizes) / sizeof(matrix_sizes[0]);

  std::cout << "======================================================================" << std::endl;
  std::cout << "        Matrix Calculation (C = aAB + bC) Performance Test " << std::endl;
  std::cout << "======================================================================" << std::endl;
#pragma omp parallel
  {
#pragma omp master
    {
      std::cout << "Running with " << omp_get_num_threads() << " OpenMP threads." << std::endl;
    }
  }
  std::cout << "----------------------------------------------------------------------" << std::endl;

  for (int i = 0; i < num_sizes; ++i)
  {
    int n = matrix_sizes[i];
    std::cout << "Testing Matrix Size: " << n << " x " << n << std::endl;

    // 1. allocate memory
    float **A = allocate_matrix(n);
    float **B = allocate_matrix(n);
    float **C_base = allocate_matrix(n);
    float **C_serial_result = allocate_matrix(n);
    float **C_parallel_result = allocate_matrix(n);

    // allocation failed
    if (!A || !B || !C_base || !C_serial_result || !C_parallel_result)
    {
      free_matrix(A);
      free_matrix(B);
      free_matrix(C_base);
      free_matrix(C_serial_result);
      free_matrix(C_parallel_result);

      std::cerr << "Error: Failed to allocate memory for matrices." << std::endl;
      return 1;
    }

    // 2. initialization
    initialize_matrices(n, A, B, C_base);

    // 3. copy & run serial test

    double start_time_serial = timestamp();
    for (int k = 0; k < ITERATIONS; ++k)
    {
      // 注意：这里我们持续在 C_serial_result 上迭代，与您最初的逻辑一致
      // 如果每次都想从头算，需要在此处 copy_matrix
      copy_matrix(n, C_serial_result, C_base);
      calc_serial(n, a, b, A, B, C_serial_result);
    }
    double end_time_serial = timestamp();
    // double time_serial = end_time_serial - start_time_serial;
    double time_serial = (end_time_serial - start_time_serial) / ITERATIONS;

    // 4. copy & run parallel test
    double start_time_parallel = timestamp();
    for (int k = 0; k < ITERATIONS; ++k)
    {
      copy_matrix(n, C_parallel_result, C_base);
      calc_parallel(n, a, b, A, B, C_parallel_result);
    }
    double end_time_parallel = timestamp();
    // double time_parallel = end_time_parallel - start_time_parallel;
    double time_parallel = (end_time_parallel - start_time_parallel) / ITERATIONS;

    // 5. Verify
    //  verify_results(n, C_serial_result, C_parallel_result);
    // We must verify the result of a SINGLE operation, not the accumulated result after ITERATIONS.
    copy_matrix(n, C_serial_result, C_base);     // Reset to base
    calc_serial(n, a, b, A, B, C_serial_result); // Perform ONE serial calculation

    copy_matrix(n, C_parallel_result, C_base);       // Reset to base
    calc_parallel(n, a, b, A, B, C_parallel_result); // Perform ONE parallel calculation

    verify_results(n, C_serial_result, C_parallel_result);

    // 6. compute and print results
    // double flops = 2 * N * N + 2 * N * N * N + 2 * N * N;
    double flops = 2.0 * n * n + 2.0 * n * n * n + 2.0 * n * n;

    double gflops_serial = flops / 1e9;
    double gflops_parallel = flops / 1e9;
    double gflops_serial_ps = flops / time_serial / 1e9;
    double gflops_parallel_ps = flops / time_parallel / 1e9;
    double speedup = time_serial / time_parallel;

    // printf("  Serial -> Time: %8.4f s | GFLOPS/s: %lf | GFLOPS/s: %8.2f\n", time_serial, gflops_serial, gflops_serial_ps);
    // printf("  OpenMP -> Time: %8.4f s | GFLOPS/s: %lf | GFLOPS/s: %8.2f\n", time_parallel, gflops_parallel, gflops_parallel_ps);

    printf("  Serial -> Time: %lf s | GFLOPS: %lf | GFLOPS/s: %lf\n", time_serial, gflops_serial, gflops_serial_ps);
    printf("  OpenMP -> Time: %lf s | GFLOPS: %lf | GFLOPS/s: %lf\n", time_parallel, gflops_parallel, gflops_parallel_ps);
    printf("  Speedup: %.2fx\n", speedup);
    std::cout << "----------------------------------------------------------------------" << std::endl;

    // 7. free memory
    free_matrix(A);
    free_matrix(B);
    free_matrix(C_base);
    free_matrix(C_serial_result);
    free_matrix(C_parallel_result);
  }

  return 0;
}