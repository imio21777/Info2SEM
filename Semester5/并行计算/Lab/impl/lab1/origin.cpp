#include <iostream>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#define N 800
#define ITERATIONS 10
using namespace std;

double timestamp()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + 1e-6 * tv.tv_usec;
}

void yourFunction(float a, float b, float A[N][N], float B[N][N], float C[N][N])
{
#pragma omp parallel for schedule(static)
  for (int j = 0; j < N; j++)
  {
    for (int i = 0; i < N; i++)
    {
      C[i][j] += b * C[i][j];
      float tmp = 0;
      for (int k = 0; k < N; k++)
      {
        tmp += A[i][k] * B[k][j];
      }
      C[i][j] += tmp * a;
    }
  }
}

int main()
{
  float A[N][N], B[N][N], C[N][N];
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

  for (int j = 0; j < N; j++)
  {
    for (int i = 0; i < N; i++)
    {
      C[i][j] += b * C[i][j];
      float tmp = 0;
      for (int k = 0; k < N; k++)
      {
        // C[i][j] += a*A[i][k]*B[k][j];
        tmp += A[i][k] * B[k][j];
      }
      C[i][j] += tmp * a;
    }
  }

  for (int j = 0; j < N; j++)
  {
    for (int i = 0; i < N; i++)
    {
      C[i][j] = 0;
    }
  }

  double time1 = timestamp();
  for (int numOfTimes = 0; numOfTimes < ITERATIONS; numOfTimes++)
  {

    yourFunction(a, b, A, B, C);
  }
  double time2 = timestamp();

  double time = (time2 - time1) / ITERATIONS;
  double flops = 2 * N * N + 2 * N * N * N + 2 * N * N;
  // double flops = 2*N*N + 2*N*N*N + N*N*N;
  double gflopsPerSecond = flops / (1000000000) / time;
  printf("GFLOPS/s=%lf\n", gflopsPerSecond);
  printf("GFLOPS=%lf\n", flops / (1000000000));
  printf("time(s)=%lf\n", time);
  return 0;
}
