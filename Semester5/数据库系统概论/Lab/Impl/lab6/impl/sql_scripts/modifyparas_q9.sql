-- 分析 Q9 查询性能
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

SET jit = off;

SHOW work_mem;
SHOW random_page_cost;
SHOW effective_cache_size;
SHOW effective_io_concurrency;
SHOW max_parallel_workers_per_gather;

-- 1. 查看查询计划 (Estimation Only)
EXPLAIN
select
	nation,
	o_year,
	sum(amount) as sum_profit
from
	(
		select
			n_name as nation,
			extract(year from o_orderdate) as o_year,
			l_extendedprice * (1 - l_discount) - ps_supplycost * l_quantity as amount
		from
			part,
			supplier,
			lineitem,
			partsupp,
			orders,
			nation
		where
			s_suppkey = l_suppkey
			and ps_suppkey = l_suppkey
			and ps_partkey = l_partkey
			and p_partkey = l_partkey
			and o_orderkey = l_orderkey
			and s_nationkey = n_nationkey
			and p_name like '%green%'
	) as profit
group by
	nation,
	o_year
order by
	nation,
	o_year desc;

-- 2. 查看实际执行计划与统计 (Execution)
EXPLAIN (ANALYZE, BUFFERS)
select
	nation,
	o_year,
	sum(amount) as sum_profit
from
	(
		select
			n_name as nation,
			extract(year from o_orderdate) as o_year,
			l_extendedprice * (1 - l_discount) - ps_supplycost * l_quantity as amount
		from
			part,
			supplier,
			lineitem,
			partsupp,
			orders,
			nation
		where
			s_suppkey = l_suppkey
			and ps_suppkey = l_suppkey
			and ps_partkey = l_partkey
			and p_partkey = l_partkey
			and o_orderkey = l_orderkey
			and s_nationkey = n_nationkey
			and p_name like '%green%'
	) as profit
group by
	nation,
	o_year
order by
	nation,
	o_year desc;
