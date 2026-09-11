-- 优化 Q5 查询 (无 CLUSTER 版)
\timing on

-- 0. 提示：建议在命令行使用 -c "SET maintenance_work_mem='2GB';"

-- 1. 创建覆盖索引 (Covering Indexes) & 复合索引
-- 核心思想：通过 INCLUDE 子句包含查询所需的所有列，实现 Index Only Scan，避免回表。

-- [Orders 优化]
-- 过滤条件：o_orderdate (范围查询)
-- Join 需求：o_custkey, o_orderkey
-- 建立复合索引：以 o_orderdate 为先导 (支持范围扫描)，包含 Join 所需列
CREATE INDEX IF NOT EXISTS idx_orders_q5_cover ON orders(o_orderdate, o_orderkey, o_custkey);

-- [Lineitem 优化] - 性能关键
-- Join 来源：orders (通过 l_orderkey)
-- Join 目标：supplier (通过 l_suppkey)
-- 计算需求：l_extendedprice, l_discount
-- 建立覆盖索引：以 l_orderkey 为先导 (支持被 Orders 驱动的 Join)，包含所有必要字段
-- 这样 Postgres 可以直接从索引中获取所有数据，无需读取任何堆页面 (Index Only Scan)
CREATE INDEX IF NOT EXISTS idx_lineitem_q5_cover ON lineitem(l_orderkey, l_suppkey) INCLUDE (l_extendedprice, l_discount);

-- [其他辅助索引]
-- Region: 过滤 'ASIA'
CREATE INDEX IF NOT EXISTS idx_region_name ON region(r_name);

-- Nation, Supplier, Customer: 外键 Join 优化
CREATE INDEX IF NOT EXISTS idx_nation_regionkey ON nation(n_regionkey);
CREATE INDEX IF NOT EXISTS idx_supplier_nationkey ON supplier(s_nationkey);
CREATE INDEX IF NOT EXISTS idx_customer_nationkey ON customer(c_nationkey);

-- 2. 更新统计信息
ANALYZE region;
ANALYZE nation;
ANALYZE supplier;
ANALYZE customer;
ANALYZE orders;
ANALYZE lineitem;

-- 3. 执行查询
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
