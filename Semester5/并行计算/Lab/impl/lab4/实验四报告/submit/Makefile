CC	= g++
NVCC	= nvcc
CFLAGS	= -std=c++11 -O3
OMPFLAGS = -fopenmp -O3 -march=native -ffast-math
NVCCFLAGS = -O3 -arch=sm_60 -std=c++11

.PHONY: all clean serial openmp cuda

all: logVS logVS_omp logVS_cuda

# Serial version
serial: logVS

logVS: main.o
	$(CC) -o $@ $^ $(CFLAGS)

main.o: main.cpp
	$(CC) -c $(CFLAGS) $<

# OpenMP version
openmp: logVS_omp

logVS_omp: main_openmp.o
	$(CC) -o $@ $^ $(CFLAGS) $(OMPFLAGS)

main_openmp.o: main_openmp.cpp
	$(CC) -c $(CFLAGS) $(OMPFLAGS) $< -o $@

# CUDA version
cuda: logVS_cuda

logVS_cuda: main_cuda.o
	$(NVCC) -o $@ $^ $(NVCCFLAGS)

main_cuda.o: main_cuda.cu
	$(NVCC) -c $(NVCCFLAGS) $< -o $@

clean: 
	rm -f *.o
	rm -f logVS logVS_omp logVS_cuda
	rm -f result.dat
