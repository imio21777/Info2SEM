-- 优化 Q9 查询 (无 CLUSTER 版)
\timing on

-- 0. 提示：建议在命令行使用 -c "SET maintenance_work_mem='2GB';"

-- 1. 创建覆盖索引 (Covering Indexes)

-- [PartSupp 优化] - 核心瓶颈
-- Start Point: Join with lineitem on (ps_partkey, ps_suppkey)
-- Data needed: ps_supplycost
-- Covering Index: 包含 Join Key 和计算字段，允许 Index Only Scan
CREATE INDEX IF NOT EXISTS idx_partsupp_q9_cover ON partsupp(ps_partkey, ps_suppkey) INCLUDE (ps_supplycost);

-- [Lineitem 优化]
-- Join with Part (l_partkey), Supplier (l_suppkey), Orders (l_orderkey)
-- Data needed: l_extendedprice, l_discount, l_quantity
-- Covering Index: 支持通过 Part 驱动的 Join，包含所有计算字段
CREATE INDEX IF NOT EXISTS idx_lineitem_q9_cover ON lineitem(l_partkey, l_suppkey, l_orderkey) INCLUDE (l_extendedprice, l_discount, l_quantity);

-- [其他索引]
-- Orders(o_orderdate) 用于 extract(year)
CREATE INDEX IF NOT EXISTS idx_orders_year ON orders(o_orderdate);
-- Supplier(s_nationkey)
CREATE INDEX IF NOT EXISTS idx_supp_nation ON supplier(s_nationkey);
-- Part(p_name): 通配符前缀 (%green%) 导致标准 B-Tree 无效，故不创建。

-- 2. 更新统计信息
ANALYZE part;
ANALYZE supplier;
ANALYZE lineitem;
ANALYZE partsupp;
ANALYZE orders;
ANALYZE nation;

-- 3. 执行查询
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
