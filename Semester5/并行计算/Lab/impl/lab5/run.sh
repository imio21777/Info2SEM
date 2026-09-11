g++ paillier_cpu.cpp -lgmp -fopenmp -O3 -o paillier_cpu
./paillier_cpu 10000 | tee paillier_cpu_output.txt

g++ ou_cpu.cpp -lgmp -fopenmp -O3 -o ou_cpu
./ou_cpu 10000 | tee ou_cpu_output.txt

nvcc -I ./CGBN/include -lgmp -o paillier_gpu paillier_gpu.cu
./paillier_gpu 10000 | tee paillier_gpu_output.txt

nvcc -I ./CGBN/include -lgmp -o ou_gpu ou_gpu.cu
./ou_gpu 10000 | tee ou_gpu_output.txt