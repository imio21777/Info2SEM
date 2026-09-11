# 实验 5：基于 GPU 的加法同态加密算法实验指南

本指南包含 Paillier 和 Okamoto-Uchiyama (OU) 两种算法的 CPU (C++) 与 GPU (CUDA) 实现的编译与运行说明。

## 1. 环境准备

需要以下环境：
*   **Linux/Ubuntu** (推荐)
*   **CUDA Toolkit**: 建议 11.0+
*   **CGBN Library**: `git clone https://github.com/NVlabs/CGBN`
*   **GMP Library**: `sudo apt-get install libgmp-dev`
*   **C++ Compiler**: g++

## 2. 源代码文件

### Paillier 算法
*   `paillier_cpu.cpp`: CPU 基准 (C++ + GMP)
*   `paillier_gpu.cu`: GPU 加速 (CUDA + CGBN)

### Okamoto-Uchiyama (OU) 算法
*   `ou_cpu.cpp`: CPU 基准 (C++ + GMP)
*   `ou_gpu.cu`: GPU 加速 (CUDA + CGBN)

## 3. 编译与运行

假设 CGBN 库位于当前目录下的 `CGBN` 文件夹（即 `./CGBN/include` 存在）。

### 3.1 编译所有程序

```bash
# 1. 编译 CPU 版本 (需链接 gmp 和 openmp)
g++ paillier_cpu.cpp -lgmp -fopenmp -O3 -o paillier_cpu
g++ ou_cpu.cpp -lgmp -fopenmp -O3 -o ou_cpu

# 2. 编译 GPU 版本 (需链接 gmp, 指定 CGBN 头文件路径)
# 请根据显卡架构调整 -arch=sm_XX, 例如 V100 用 sm_70, A100 用 sm_80
nvcc -I ./CGBN/include -lgmp -o paillier_gpu paillier_gpu.cu
nvcc -I ./CGBN/include -lgmp -o ou_gpu ou_gpu.cu
```

### 3.2 运行测试

**Step 1: 运行 CPU 基准**
```bash
./paillier_cpu 100
./ou_cpu 100
```
*   观察输出的平均加密/解密时间（毫秒）。
*   OU 的解密通常比 Paillier 快，因为模数更小且计算复杂度低。

**Step 2: 运行 GPU 加速**
```bash
./paillier_gpu 1000
./ou_gpu 1000
```
*   观察 GPU 在大批量（如 1000 或 10000）下的吞吐量优势。
*   确认 `Verification errors: 0`。

## 4. 预期结果对比

| 算法 | 设备 | 操作 | 单次耗时 (预估) | 吞吐量 (Ops/sec) |
| :--- | :--- | :--- | :--- | :--- |
| Paillier | CPU | Encrypt | ~15 ms | ~60 |
| Paillier | GPU | Encrypt | ~0.005 ms (摊销) | ~200,000 |
| OU | CPU | Decrypt | ~4 ms | ~250 |
| OU | GPU | Decrypt | ~0.001 ms (摊销) | ~1,000,000 |

**注**：OU 算法由于解密过程主要在 $Z_p$ 而非 $Z_{n^2}$ 上进行（且 $p \ll n$），因此解密速度主要快于 Paillier。GPU 版本将进一步放大这一优势。
