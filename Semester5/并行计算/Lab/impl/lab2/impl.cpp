#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include </opt/homebrew/Cellar/libomp/21.1.0/include/omp.h> // 引入OpenMP头文件

#define N 729
#define reps 100

typedef struct time_struct
{
  double start;
  double end;
} time_str;

double a[N][N], b[N][N], c[N];
int jmax[N];

// when lo = 0, hi = N, treat as upper triangular matrix
// 负载递减，当i增大时，内层循环j的迭代次数减少
void loop1chunk(int lo, int hi)
{
  int i, j;
  for (i = lo; i < hi; i++)
  {
    for (j = N - 1; j > i; j--)
    {
      a[i][j] += cos(b[i][j]);
    }
  }
}

void loop1chunk_static(int lo, int hi)
{
  int i, j;
#pragma omp parallel for schedule(static) private(j)
  for (i = lo; i < hi; i++)
  {
    for (j = N - 1; j > i; j--)
    {
      a[i][j] += cos(b[i][j]);
    }
  }
}

void loop1chunk_dynamic(int lo, int hi)
{
  int i, j;
#pragma omp parallel for schedule(dynamic) private(j)
  for (i = lo; i < hi; i++)
  {
    for (j = N - 1; j > i; j--)
    {
      a[i][j] += cos(b[i][j]);
    }
  }
}

void loop1chunk_guided(int lo, int hi)
{
  int i, j;
#pragma omp parallel for schedule(guided) private(j)
  for (i = lo; i < hi; i++)
  {
    for (j = N - 1; j > i; j--)
    {
      a[i][j] += cos(b[i][j]);
    }
  }
}

// 负载系数不均，取决于 jmax 的取值，由后续知道 1 的时候计算量很小，但 N 的时候计算量很大
// 但随着 i 增大，jmax 取 N 的次数越来越少
void loop2chunk(int lo, int hi)
{
  int i, j, k;
  double rN2;

  rN2 = 1.0 / (double)(N * N);

  for (i = lo; i < hi; i++)
  {
    for (j = 0; j < jmax[i]; j++)
    {
      for (k = 0; k < j; k++)
      {
        c[i] += (k + 1) * log(b[i][j]) * rN2;
      }
    }
  }
}

void loop2chunk_static(int lo, int hi)
{
  int i, j, k;
  double rN2;

  rN2 = 1.0 / (double)(N * N);

#pragma omp parallel for schedule(static) private(j, k)
  for (i = lo; i < hi; i++)
  {
    for (j = 0; j < jmax[i]; j++)
    {
      for (k = 0; k < j; k++)
      {
        c[i] += (k + 1) * log(b[i][j]) * rN2;
      }
    }
  }
}

void loop2chunk_dynamic(int lo, int hi)
{
  int i, j, k;
  double rN2;

  rN2 = 1.0 / (double)(N * N);

#pragma omp parallel for schedule(dynamic) private(j, k)
  for (i = lo; i < hi; i++)
  {
    for (j = 0; j < jmax[i]; j++)
    {
      for (k = 0; k < j; k++)
      {
        c[i] += (k + 1) * log(b[i][j]) * rN2;
      }
    }
  }
}

void loop2chunk_guided(int lo, int hi)
{
  int i, j, k;
  double rN2;

  rN2 = 1.0 / (double)(N * N);

#pragma omp parallel for schedule(guided) private(j, k)
  for (i = lo; i < hi; i++)
  {
    for (j = 0; j < jmax[i]; j++)
    {
      for (k = 0; k < j; k++)
      {
        c[i] += (k + 1) * log(b[i][j]) * rN2;
      }
    }
  }
}

// initialize a as all 0.0
// initialize b as (i+j)*pi
void init1(void)
{
  int i, j;

  for (i = 0; i < N; i++)
  {
    for (j = 0; j < N; j++)
    {
      a[i][j] = 0.0;
      b[i][j] = 3.142 * (i + j);
    }
  }
}

// initialize c as all 0.0
// initialize jmax as N or 1 && N is less and less when i increases
// initialize b as (i*j+1)/(N*N)
void init2(void)
{
  int i, j, expr;

  for (i = 0; i < N; i++)
  {
    // expr = i % 1 when i in [0,29]
    // expr = i % 4 when i in [30,59]
    // expr = i % 7 when i in [60,89]
    // ...
    expr = i % (3 * (i / 30) + 1);
    if (expr == 0)
    {
      jmax[i] = N;
    }
    else
    {
      jmax[i] = 1;
    }
    c[i] = 0.0;
  }

  for (i = 0; i < N; i++)
  {
    for (j = 0; j < N; j++)
    {
      b[i][j] = (double)(i * j + 1) / (double)(N * N);
    }
  }
}

// check sum of a
void valid1(void)
{
  int i, j;
  double suma;

  suma = 0.0;
  for (i = 0; i < N; i++)
  {
    for (j = 0; j < N; j++)
    {
      suma += a[i][j];
    }
  }
  printf("Loop 1 check: Sum of a is %lf\n", suma);
}

// check sum of c
void valid2(void)
{
  int i;
  double sumc;

  sumc = 0.0;
  for (i = 0; i < N; i++)
  {
    sumc += c[i];
  }
  printf("Loop 2 check: Sum of c is %f\n", sumc);
}

int main()
{
  time_str l1, l2, l3, l4, l5, l6;
  time_str serial_l1, serial_l2;

  int lo = 0;
  int hi = N;

  printf("Starting OpenMP scheduling experiment...\n");
  printf("Problem size N = %d, Repetitions = %d\n", N, reps);
  printf("Using up to %d threads.\n\n", omp_get_max_threads());

  // =======================================================================
  // 实验部分 1: 测试 loop1 (递减负载)
  // =======================================================================
  printf("--- Testing Loop 1 (Decreasing Workload) ---\n");

  init1();
  serial_l1.start = omp_get_wtime();
  for (int r = 0; r < reps; r++)
  {
    loop1chunk(lo, hi);
  }
  serial_l1.end = omp_get_wtime();
  valid1();
  printf("Total time for %d reps of serial loop 1 = %f\n", reps, (float)(serial_l1.end - serial_l1.start));

  // --- 1.1 Static Scheduling ---
  init1();
  l1.start = omp_get_wtime();

  for (int r = 0; r < reps; r++)
  {
    loop1chunk_static(lo, hi);
  }

  l1.end = omp_get_wtime();
  valid1();

  printf("Total time for %d reps of schedule(static) loop 1 = %f\n", reps, (float)(l1.end - l1.start));

  // --- 1.2 Dynamic Scheduling ---
  init1();
  l2.start = omp_get_wtime();

  for (int r = 0; r < reps; r++)
  {
    loop1chunk_dynamic(lo, hi);
  }

  l2.end = omp_get_wtime();
  valid1();

  printf("Total time for %d reps of schedule(dynamic) loop 1 = %f\n", reps, (float)(l2.end - l2.start));

  // --- 1.3 Guided Scheduling ---
  init1();
  l3.start = omp_get_wtime();

  for (int r = 0; r < reps; r++)
  {
    loop1chunk_guided(lo, hi);
  }

  l3.end = omp_get_wtime();
  valid1();
  printf("Total time for %d reps of schedule(guided) loop 1 = %f\n", reps, (float)(l3.end - l3.start));

  printf("\n");

  // =======================================================================
  // 实验部分 2: 测试 loop2 (不规则负载)
  // =======================================================================
  printf("--- Testing Loop 2 (Irregular Workload) ---\n");

  init2();
  serial_l2.start = omp_get_wtime();
  for (int r = 0; r < reps; r++)
  {
    loop2chunk(lo, hi);
  }
  serial_l2.end = omp_get_wtime();
  valid2();
  printf("Total time for %d reps of serial loop 2 = %f\n", reps, (float)(serial_l2.end - serial_l2.start));

  // --- 2.1 Static Scheduling ---
  init2();
  l4.start = omp_get_wtime();

  for (int r = 0; r < reps; r++)
  {
    loop2chunk_static(lo, hi);
  }

  l4.end = omp_get_wtime();
  valid2();
  printf("Total time for %d reps of schedule(static) loop 2 = %f\n", reps, (float)(l4.end - l4.start));

  // --- 2.2 Dynamic Scheduling ---
  init2();
  l5.start = omp_get_wtime();

  for (int r = 0; r < reps; r++)
  {
    loop2chunk_dynamic(lo, hi);
  }

  l5.end = omp_get_wtime();
  valid2();
  printf("Total time for %d reps of schedule(dynamic) loop 2 = %f\n", reps, (float)(l5.end - l5.start));

  // --- 2.3 Guided Scheduling ---
  init2();
  l6.start = omp_get_wtime();

  for (int r = 0; r < reps; r++)
  {
    loop2chunk_guided(lo, hi);
  }

  l6.end = omp_get_wtime();
  valid2();
  printf("Total time for %d reps of schedule(guided) loop 2 = %f\n", reps, (float)(l6.end - l6.start));

  return 0;
}
