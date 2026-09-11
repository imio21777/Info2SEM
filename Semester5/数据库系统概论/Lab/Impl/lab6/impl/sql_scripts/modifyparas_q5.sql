-- 分析 Q5 查询性能
\timing on

-- 1. work_mem: 增加每个操作（排序、哈希）可用的内存
-- 默认通常只有 4MB，对于 TPC-H 这种复杂查询，增加到 256MB 可以避免磁盘排序
SET work_mem = '256MB';

-- 2. random_page_cost: 调整优化器对随机 I/O 的成本估算
-- 默认是 4.0 (HDD)，SSD 建议调整为 1.1，鼓励使用索引
SET random_page_cost = 1.1;

-- 3. effective_cache_size: 告诉优化器系统有多少内存可用作文件缓存
-- 这不会实际分配内存，只是帮助优化器做决策
SET effective_cache_size = '12GB';

-- 4. effective_io_concurrency: 设置并发 I/O 能力 (SSD)
SET effective_io_concurrency = 200;

SET max_parallel_workers_per_gather = 16;

SHOW work_mem;
SHOW random_page_cost;
SHOW effective_cache_size;
SHOW effective_io_concurrency;
SHOW max_parallel_workers_per_gather;

-- 1. 查看查询计划 (Estimation Only)
EXPLAIN
select
	n_name,
	sum(l_extendedprice * (1 - l_discount)) as revenue
from
	customer,
	orders,
	lineitem,
	supplier,
	nation,
	region
where
	c_custkey = o_custkey
	and l_orderkey = o_orderkey
	and l_suppkey = s_suppkey
	and c_nationkey = s_nationkey
	and s_nationkey = n_nationkey
	and n_regionkey = r_regionkey
	and r_name = 'ASIA'
	and o_orderdate >= date '1994-01-01'
	and o_orderdate < date '1994-01-01' + interval '1' year
group by
	n_name
order by
	revenue desc;

-- 2. 查看实际执行计划与统计 (Execution)
EXPLAIN (ANALYZE, BUFFERS)
select
	n_name,
	sum(l_extendedprice * (1 - l_discount)) as revenue
from
	customer,
	orders,
	lineitem,
	supplier,
	nation,
	region
where
	c_custkey = o_custkey
	and l_orderkey = o_orderkey
	and l_suppkey = s_suppkey
	and c_nationkey = s_nationkey
	and s_nationkey = n_nationkey
	and n_regionkey = r_regionkey
	and r_name = 'ASIA'
	and o_orderdate >= date '1994-01-01'
	and o_orderdate < date '1994-01-01' + interval '1' year
group by
	n_name
order by
	revenue desc;
