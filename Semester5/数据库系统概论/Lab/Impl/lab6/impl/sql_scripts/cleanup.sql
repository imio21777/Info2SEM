-- 清理脚本：删除实验中创建的所有索引
--用于重置实验环境
-- 使用方法: psql -U leap -d tpchdb -f impl/cleanup.sql

-- 清除 Cluster 配置 (Metadata)
-- 注意：这只会移除"聚簇"标记。Physical Table Rewrite 造成的物理顺序优化是永久的，
-- 除非重新导入数据或进行大规模更新，否则原始查询可能会因为数据的物理有序性而变快。
ALTER TABLE orders SET WITHOUT CLUSTER;
ALTER TABLE lineitem SET WITHOUT CLUSTER;

-- Q5 相关索引 (optimize_q5.sql)
DROP INDEX IF EXISTS idx_region_name;
DROP INDEX IF EXISTS idx_orders_q5_cover; -- Q5 覆盖索引
DROP INDEX IF EXISTS idx_orders_date;     -- 旧版本索引(保留以防万一)
DROP INDEX IF EXISTS idx_orders_custkey;
DROP INDEX IF EXISTS idx_orders_date_cust; -- 旧版本索引
DROP INDEX IF EXISTS idx_customer_nationkey;
DROP INDEX IF EXISTS idx_supplier_nationkey;
DROP INDEX IF EXISTS idx_nation_regionkey;
DROP INDEX IF EXISTS idx_lineitem_q5_cover; -- Q5 覆盖索引
DROP INDEX IF EXISTS idx_lineitem_orderkey;
DROP INDEX IF EXISTS idx_lineitem_suppkey;

-- Q9 相关索引 (optimize_q9.sql)
DROP INDEX IF EXISTS idx_partsupp_q9_cover; -- Q9 覆盖索引
DROP INDEX IF EXISTS idx_lineitem_q9_cover; -- Q9 覆盖索引
DROP INDEX IF EXISTS idx_partsupp_part_supp;
DROP INDEX IF EXISTS idx_lineitem_part_supp;
DROP INDEX IF EXISTS idx_orders_orderdate;
DROP INDEX IF EXISTS idx_orders_year;
DROP INDEX IF EXISTS idx_supp_nation;
DROP INDEX IF EXISTS idx_part_name; 

-- 验证清理结果：列出所有用户定义的索引
-- 应该只会看到 Primary Keys (pk_*)，不应看到上述 idx_*
\d lineitem
\d orders
\d customer
\d supplier
\d partsupp
\d nation
\d region
\d part
