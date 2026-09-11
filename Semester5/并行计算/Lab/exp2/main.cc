#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h> 

#define N 729
#define reps 100

typedef struct time_struct
{
  double start;
  double end;
} time_str;

double a[N][N], b[N][N], c[N];
int jmax[N];

void loop1chunk(int lo, int hi) {
  int i,j;
  for (i=lo; i<hi; i++){
    for (j=N-1; j>i; j--){
      a[i][j] += cos(b[i][j]);
    }
  }
}

void loop2chunk(int lo, int hi) {
  int i,j,k;
  double rN2;

  rN2 = 1.0 / (double) (N*N);

  for (i=lo; i<hi; i++){
    for (j=0; j < jmax[i]; j++){
      for (k=0; k<j; k++){
        c[i] += (k+1) * log (b[i][j]) * rN2;
      }
    }
  }

}

void init1(void){
  int i,j;


  for (i=0; i<N; i++){
    for (j=0; j<N; j++){
      a[i][j] = 0.0;
      b[i][j] = 3.142*(i+j);
    }
  }
}

void init2(void){
  int i,j, expr;

  for (i=0; i<N; i++){
    expr =  i%( 3*(i/30) + 1);
    if ( expr == 0) {
      jmax[i] = N;
    }
    else {
      jmax[i] = 1;
    }
    c[i] = 0.0;
  }

  for (i=0; i<N; i++){
    for (j=0; j<N; j++){
      b[i][j] = (double) (i*j+1) / (double) (N*N);
    }
  }
}

void valid1(void) {
  int i,j;
  double suma;

  suma= 0.0;
  for (i=0; i<N; i++){
    for (j=0; j<N; j++){
      suma += a[i][j];
    }
  }
  printf("Loop 1 check: Sum of a is %lf\n", suma);
}


void valid2(void) {
  int i;
  double sumc;

  sumc= 0.0;
  for (i=0; i<N; i++){
    sumc += c[i];
  }
  printf("Loop 2 check: Sum of c is %f\n", sumc);
}

int main(){
  time_str l1,l2;


  init1();


  l1.start = omp_get_wtime(); 

  int lo = 0;
  int hi = N;

  for (int r=0; r<reps; r++){
    loop1chunk(lo, hi);
  }

  l1.end  = omp_get_wtime();
  valid1();

  printf("Total time for %d reps of loop 1 = %f\n",reps, (float)(l1.end-l1.start));

  init2(); 


  l2.start = omp_get_wtime(); 

  lo = 0;
  hi = N;

  for (int r=0; r<reps; r++){
    loop2chunk(lo, hi);
  }

  l2.end  = omp_get_wtime();

  valid2();


  printf("Total time for %d reps of loop 2 = %f\n",reps, (float)(l2.end-l2.start));


  return 0;
}
