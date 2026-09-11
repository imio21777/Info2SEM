# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个基于文件系统的商城库存管理系统，使用C++实现。系统支持商品管理、库存操作、记录查询和数据汇总等功能。

## 开发命令

### 编译和运行
```bash
# 编译项目
make

# 清理编译产物
make clean

# 编译并运行
make run

# 直接运行
./inventory_system
```

## 系统架构

### 目录结构
- `include/inventory.h`: 核心类和数据结构定义
- `src/inventory.cpp`: 业务逻辑实现
- `src/main.cpp`: 主程序和用户界面
- `data/products.txt`: 商品信息存储文件
- `data/transactions.txt`: 进销记录存储文件
- `Makefile`: 构建配置

### 核心数据结构
- `Product`: 商品信息结构体，包含ID、名称、类别、价格、库存和删除标记
- `Transaction`: 交易记录结构体，包含记录ID、商品ID、操作类型、操作人、时间戳和数量
- `InventoryManager`: 核心管理类，负责所有业务逻辑

### 文件存储格式
- **products.txt**: 每行一个商品，字段用空格分隔：`ID 名称 类别 价格 库存 是否删除`
- **transactions.txt**: 每行一个记录，字段用空格分隔：`记录ID 商品ID 操作类型 操作人 时间戳 数量`

### 核心功能模块
1. **商品管理**: 添加、删除、查看商品目录
2. **库存操作**: 进货和销售操作，自动更新库存
3. **记录管理**: 所有操作自动记录，支持查询和筛选
4. **数据汇总**: 按时间范围和类别统计销量
5. **分类浏览**: 按商品类别组织显示，支持库存排序

## 重要设计原则

- 所有数据持久化到文本文件，确保数据不丢失
- 商品删除采用软删除，保留历史记录
- 时间格式统一为 `YYYY-MM-DD HH:MM:SS`
- 输入验证确保数据完整性和正确性
- 使用STL容器和算法提高代码效率