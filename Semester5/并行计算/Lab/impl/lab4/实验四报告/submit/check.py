#!/usr/bin/env python3
"""
结果验证脚本
用于检查并行版本的计算结果是否在允许误差范围内
"""

import sys

def check(file1, file2, tolerance=1e-5):
    """
    比较两个结果文件，检查数值误差
    
    Args:
        file1: 第一个结果文件路径
        file2: 第二个结果文件路径（通常是 check.dat）
        tolerance: 允许的相对误差（默认 1e-5）
    
    Returns:
        bool: 验证是否通过
    """
    try:
        with open(file1, 'r') as f1, open(file2, 'r') as f2:
            lines1 = f1.readlines()
            lines2 = f2.readlines()
    except FileNotFoundError as e:
        print(f"❌ 错误: 文件未找到 - {e}")
        return False
    
    if len(lines1) != len(lines2):
        print(f"❌ 行数不匹配: {len(lines1)} vs {len(lines2)}")
        return False
    
    max_error = 0.0
    error_count = 0
    error_lines = []
    
    for i, (line1, line2) in enumerate(zip(lines1, lines2)):
        line1 = line1.strip()
        line2 = line2.strip()
        
        if not line1 or not line2:
            continue
        
        try:
            parts1 = line1.split(': ')
            parts2 = line2.split(': ')
            
            if len(parts1) != 2 or len(parts2) != 2:
                print(f"⚠️  警告: 第 {i+1} 行格式不正确")
                continue
            
            idx1, val1 = parts1[0], float(parts1[1])
            idx2, val2 = parts2[0], float(parts2[1])
            
            if idx1 != idx2:
                print(f"❌ 索引不匹配 at line {i+1}: {idx1} vs {idx2}")
                return False
            
            # 计算相对误差
            if abs(val2) > 1e-10:
                error = abs(val1 - val2) / abs(val2)
            else:
                error = abs(val1 - val2)
            
            max_error = max(max_error, error)
            
            if error > tolerance:
                error_count += 1
                if error_count <= 5:  # 只记录前 5 个错误
                    error_lines.append({
                        'line': i+1,
                        'error': error,
                        'val1': val1,
                        'val2': val2
                    })
        
        except ValueError as e:
            print(f"⚠️  警告: 第 {i+1} 行解析失败 - {e}")
            continue
    
    # 输出结果
    print(f"\n{'='*60}")
    print(f" 验证结果")
    print(f"{'='*60}")
    print(f"总行数: {len(lines1)}")
    print(f"最大相对误差: {max_error:.2e}")
    print(f"超出容差的数量: {error_count}/{len(lines1)}")
    
    if error_lines:
        print(f"\n前 {len(error_lines)} 个错误:")
        print(f"{'-'*60}")
        for err in error_lines:
            print(f"  行 {err['line']:6d}: 误差 = {err['error']:.2e}")
            print(f"              值1 = {err['val1']:.10f}")
            print(f"              值2 = {err['val2']:.10f}")
    
    print(f"{'='*60}\n")
    
    if max_error <= tolerance:
        print("✅ 验证通过！所有结果在允许误差范围内。")
        return True
    else:
        print(f"❌ 验证失败：最大误差 {max_error:.2e} 超出容差 {tolerance:.2e}")
        return False

def main():
    if len(sys.argv) != 3:
        print("用法: python3 check.py <result_file> <check_file>")
        print("示例: python3 check.py result.dat check.dat")
        sys.exit(1)
    
    result_file = sys.argv[1]
    check_file = sys.argv[2]
    
    print(f"{'='*60}")
    print(f" 正在验证结果文件")
    print(f"{'='*60}")
    print(f"结果文件: {result_file}")
    print(f"标准文件: {check_file}")
    print(f"允许误差: 1e-5 (0.001%)")
    print(f"{'='*60}\n")
    
    success = check(result_file, check_file)
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
