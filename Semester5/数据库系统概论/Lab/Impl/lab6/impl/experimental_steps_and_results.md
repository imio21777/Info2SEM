# 实验步骤与预期结果指南

本文档详细说明了如何执行本实验的各个步骤，以及每一步预期的执行结果（包括 Execution Plan 的关键变化）。

## 前置准备 (Environment Setup)

确保已进入 PostgreSQL 命令行或使用图形化客户端连接到 `tpchdb` 数据库。
确保 `impl/` 目录下的 SQL 脚本文件已就绪。

---

## 任务 1: 分析查询性能瓶颈 (Analyze Query Performance)

### 步骤 1.1: 分析 Q5 (某地区供货商收入)

**执行命令:**
```bash
psql -U leap -d tpchdb -f impl/analysis_q5.sql | tee impl/analysis_q5_output.txt
```

**预期结果 (Expected Output):**
执行完成后，请查看生成的 **`impl/analysis_q5_output.txt`** 文件。
1.  **纯执行计划 (EXPLAIN Estimation)**:
    -   首先脚本会输出仅基于统计信息的预测计划。
    -   重点关注 `Estimated Startup Cost` 和 `Total Cost`。此时数据库并不会执行查询。
2.  **实际执行统计 (EXPLAIN ANALYZE Execution)**:
    -   随后输出包含 `Actual Time` 和 `Buffers` 的真实执行情况。
    -   你会看到 `Seq Scan on lineitem` (全表扫描 Lineitem 表)，因为没有索引。
    -   你会看到 `Hash Join`。由于参与连接的表较大，PostgreSQL 倾向于使用哈希连接。
    -   代价 (Cost) 较高，例如 `cost=0.00..35890.00` (具体数值取决于数据量)。
2.  **瓶颈分析**:
    -   `lineitem` 表是最大的表，全表扫描消耗了大部分 I/O。
    -   内存 (`work_mem`) 可能不足以容纳整个 Hash Table，导致 "Batching" 或磁盘交换。

### 步骤 1.2: 分析 Q9 (产品类型利润)

**执行命令:**
```bash
psql -U leap -d tpchdb -f impl/analysis_q9.sql | tee impl/analysis_q9_output.txt
```

**预期结果 (Expected Output):**
执行完成后，请查看生成的 **`impl/analysis_q9_output.txt`** 文件。
1.  **执行计划**:
    -   `Seq Scan on part`，过滤条件 `p_name like '%green%'`。由于 `%` 在前面，标准索引无效。
    -   `Seq Scan on lineitem` 与 `partsupp` 的连接。
2.  **瓶颈分析**:
    -   多表的大规模 Join (Part, Supplier, Lineitem, Partsupp, Orders, Nation) 极其耗时。
    -   缺乏连接键 (Join Key) 的索引导致数据库必须扫描整个表来寻找匹配行。

---

## 任务 2: 使用 SQL 优化技术 (Query Optimization)

### 步骤 2.1: 优化 Q5

**操作**:
我们将在外键 (`l_orderkey`, `l_suppkey`, `c_nationkey` 等) 上建立 B-Tree 索引。

**执行命令:**
```bash
# 推荐使用 -c 临时增加内存加速索引创建
psql -U leap -d tpchdb -c "SET maintenance_work_mem='2GB';" -f impl/optimize_q5.sql | tee impl/optimize_q5_output.txt
```

**预期结果 (Expected Output):**
执行完成后，请查看 **`impl/optimize_q5_output.txt`**。
1.  **索引创建**: 终端显示 `CREATE INDEX` 成功。
2.  **新执行计划**:
    -   **`Seq Scan` 变为 `Index Scan` 或 `Bitmap Heap Scan`**。例如 `Index Scan using idx_lineitem_orderkey on lineitem`。
    -   **Join 方式可能改变**: 可能会看到 `Nested Loop` 或更高效的 `Merge Join`，因为索引提供了排序顺序或快速查找。
3.  **性能提升**:
    -   执行时间 (Execution Time) 应显著下降。
    -   **预期耗时**: 从 ~1500ms 降至 ~300ms (基于 1GB 数据估算)。
    -   **提升幅度**: > 70%。

### 步骤 2.2: 优化 Q9

**操作**:
在 `partsupp(ps_partkey, ps_suppkey)` 上建立复合索引，以及其他关联键索引。

**执行命令:**
```bash
# 推荐使用 -c 临时增加内存加速索引创建
psql -U leap -d tpchdb -c "SET maintenance_work_mem='2GB';" -f impl/optimize_q9.sql | tee impl/optimize_q9_output.txt
```

**预期结果 (Expected Output):**
执行完成后，请查看 **`impl/optimize_q9_output.txt`**。
1.  **新执行计划**:
    -   `partsupp` 的访问将变为 `Index Scan using idx_partsupp_part_supp`。这非常关键，因为 Q9 需要同时匹配 part 和 supplier。
    -   虽然 `p_name like '%green%'` 仍然可能全表扫描 `part`，但后续的 Join 代价大幅降低。
2.  **性能提升**:
    -   **预期耗时**: 从 ~4500ms 降至 ~1200ms。
    -   **提升幅度**: > 70%。

---

## 任务 3: 数据库参数调优 (Parameter Tuning)

**操作**:
修改 `postgresql.conf` 文件（或在当前 Session 中应用参数）。我们的脚本 `impl/postgres_tuning.conf` 列出了推荐值。

**主要调整**:
1.  `work_mem = 256MB` (增加排序和哈希可用的内存)
2.  `random_page_cost = 1.1` (告诉优化器随机 I/O 很便宜，鼓励用索引)
3.  `shared_buffers = 4GB` (缓存更多数据)

### 方式一：Session 级别测试 (推荐实验使用)
这是最简单不需要重启数据库的方法，可以立即验证效果。
但请注意：`shared_buffers` **无法**通过这种方式修改，那是需要在启动时分配内存的参数。

**操作步骤**:
1.  在 SQL 客户端中打开一个新的查询窗口。
2.  运行或者粘贴 `impl/apply_parameters_session.sql` 中的内容。
    ```sql
    SET work_mem = '256MB';
    SET random_page_cost = 1.1;
    ```
3.  **在同一个窗口/会话内**，紧接着运行 Q9 的分析脚本 (如复制 `impl/optimize_q9.sql` 的内容运行)。
    *注意：如果关闭窗口或断开连接，参数会恢复默认。*

### 方式二：修改配置文件 (生产环境做法)
如果您有权限修改数据库服务器的配置文件，可以永久生效所有参数（包括 `shared_buffers`）。

**操作步骤**:
1.  找到 PostgreSQL 的数据目录 (通常用 `SHOW data_directory;` 查看)。
2.  找到 `postgresql.conf` 文件。
3.  使用文本编辑器打开它，将 `impl/postgres_tuning.conf` 中的内容追加到末尾，或者修改对应行。
4.  **重启 PostgreSQL 服务** (必须重启才能让 `shared_buffers` 生效)。

**验证**:
重启后运行 `SHOW shared_buffers;` 确认数值已变更为 4GB。


---

## 任务 4: 综合优化 (Combined Optimization)

**总结**:
通过结合 **索引优化** (减少读取的数据量) 和 **参数调优** (提高处理数据的速度)，我们实现了性能的最大化。

**最终预期数据 (1GB TPC-H)**:
-   **Q5**: ~200ms (原 ~1.5s)
-   **Q9**: ~800ms (原 ~4.5s)
