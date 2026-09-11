#!/bin/bash
# 高效完整测试：每个版本运行一次，同时完成性能测试和正确性验证
# 用于自动验证结果并收集性能数据

echo "========================================="
echo " FTT 性能测试 + 正确性验证"

# 检查可执行文件
if [ ! -f "./logVS" ]; then
    echo "❌ 错误: logVS 不存在，请先编译"
    exit 1
fi

if [ ! -f "./logVS_omp" ]; then
    echo "⚠️  警告: logVS_omp 不存在，跳过 OpenMP 测试"
    SKIP_OMP=1
fi

if [ ! -f "./logVS_cuda" ]; then
    echo "⚠️  警告: logVS_cuda 不存在，跳过 CUDA 测试"
    SKIP_CUDA=1
fi

# 检查验证工具
VERIFY=1
if [ ! -f "./check.py" ] || [ ! -f "check.dat" ]; then
    echo "⚠️  警告: 缺少验证文件，跳过正确性验证"
    VERIFY=0
fi

################################
# 1. 串行版本
################################

echo "========================================="
echo " 1. 串行版本 (基准)"
echo "========================================="

# 运行并记录时间
SERIAL_TIME=$(./logVS 2>&1 | grep "Computing time" | awk -F'=' '{print $2}' | awk '{print $1}')
SERIAL_SEC=$(echo "scale=2; $SERIAL_TIME / 1000000" | bc)

echo "运行时间: $SERIAL_SEC 秒 ($SERIAL_TIME μs)"

# 验证正确性
if [ $VERIFY -eq 1 ]; then
    if python3 check.py result.dat check.dat > /dev/null 2>&1; then
        echo "正确性: ✅ 通过"
        SERIAL_CORRECT=1
    else
        echo "正确性: ❌ 失败"
        SERIAL_CORRECT=0
    fi
fi

#############################################
# 2. OpenMP 版本
#############################################

if [ -z "$SKIP_OMP" ]; then
    echo "========================================="
    echo " 2. OpenMP 版本"
    echo "========================================="
    
    printf "%-8s | %-10s | %-10s | %-8s\n" "线程数" "时间(秒)" "加速比" "正确性"
    echo "-----------------------------------------"
    
    for threads in 1 2 4 8 16 32; do
        export OMP_NUM_THREADS=$threads
        
        # 运行并记录时间
        OMP_TIME=$(./logVS_omp 2>&1 | grep "Computing time" | awk -F'=' '{print $2}' | awk '{print $1}')
        
        if [ -n "$OMP_TIME" ] && [ -n "$SERIAL_TIME" ]; then
            OMP_SEC=$(echo "scale=2; $OMP_TIME / 1000000" | bc)
            SPEEDUP=$(echo "scale=2; $SERIAL_TIME / $OMP_TIME" | bc)
            EFFICIENCY=$(echo "scale=1; ($SPEEDUP / $threads) * 100" | bc)
            
            # 验证正确性
            if [ $VERIFY -eq 1 ]; then
                if python3 check.py result.dat check.dat > /dev/null 2>&1; then
                    CORRECT="✅"
                else
                    CORRECT="❌"
                fi
            else
                CORRECT="N/A"
            fi
            
            printf "%-8d | %-10s | %-10s | %-8s\n" \
                   $threads "$OMP_SEC" "${SPEEDUP}x" "$CORRECT"
            
            # 记录 32 线程的结果
            if [ $threads -eq 32 ]; then
                BEST_OMP_TIME=$OMP_TIME
                BEST_OMP_SEC=$OMP_SEC
                BEST_OMP_SPEEDUP=$SPEEDUP
                if [ "$CORRECT" = "✅" ]; then
                    OMP_CORRECT=1
                else
                    OMP_CORRECT=0
                fi
            fi
        else
            printf "%-8d | %-10s | %-10s | %-8s\n" \
                   $threads "失败" "-" "❌"
        fi
    done
fi

#############################################
# 3. CUDA 版本
#############################################

if [ -z "$SKIP_CUDA" ]; then
    echo "========================================="
    echo " 3. CUDA 版本"
    echo "========================================="
    
    # 运行并记录时间
    CUDA_TIME=$(./logVS_cuda 2>&1 | grep "Computing time" | awk -F'=' '{print $2}' | awk '{print $1}')
    
    if [ -n "$CUDA_TIME" ] && [ -n "$SERIAL_TIME" ]; then
        CUDA_SEC=$(echo "scale=2; $CUDA_TIME / 1000000" | bc)
        CUDA_SPEEDUP=$(echo "scale=2; $SERIAL_TIME / $CUDA_TIME" | bc)
        
        echo "运行时间: $CUDA_SEC 秒 ($CUDA_TIME μs)"
        echo "加速比 (vs 串行): ${CUDA_SPEEDUP}x"
        
        if [ -n "$BEST_OMP_TIME" ]; then
            CUDA_VS_OMP=$(echo "scale=2; $BEST_OMP_TIME / $CUDA_TIME" | bc)
            echo "加速比 (vs OpenMP 32核): ${CUDA_VS_OMP}x"
        fi
        
        # 验证正确性
        if [ $VERIFY -eq 1 ]; then
            if python3 check.py result.dat check.dat > /dev/null 2>&1; then
                echo "正确性: ✅ 通过"
                CUDA_CORRECT=1
            else
                echo "正确性: ❌ 失败"
                CUDA_CORRECT=0
            fi
        fi
    else
        echo "❌ CUDA 版本运行失败"
    fi
fi

#############################################
# 总结报告
#############################################

echo "========================================="
echo " 测试总结"
echo "========================================="

# 正确性汇总
if [ $VERIFY -eq 1 ]; then
    echo "【正确性验证】"
    [ "$SERIAL_CORRECT" = "1" ] && echo "  串行:   ✅ 通过" || echo "  串行:   ❌ 失败"
    [ -z "$SKIP_OMP" ] && { [ "$OMP_CORRECT" = "1" ] && echo "  OpenMP: ✅ 通过" || echo "  OpenMP: ❌ 失败"; }
    [ -z "$SKIP_CUDA" ] && { [ "$CUDA_CORRECT" = "1" ] && echo "  CUDA:   ✅ 通过" || echo "  CUDA:   ❌ 失败"; }
    echo ""
fi

# 性能汇总
echo "【性能对比】"
echo "  串行:      $SERIAL_SEC 秒 (基准 1.0x)"
[ -n "$BEST_OMP_SEC" ] && echo "  OpenMP:    $BEST_OMP_SEC 秒 (${BEST_OMP_SPEEDUP}x 加速)"
[ -n "$CUDA_SEC" ] && echo "  CUDA:      $CUDA_SEC 秒 (${CUDA_SPEEDUP}x 加速)"
echo ""
echo "========================================="
echo " 测试完成！"
echo "========================================="
