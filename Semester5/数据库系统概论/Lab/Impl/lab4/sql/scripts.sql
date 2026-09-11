-- 实验四 安全性实验 SQL 脚本

-- ===========================
-- 1. 教材第四章示例（多用户权限变更观察）
-- ===========================

-- 1.1 创建示例用户与基础对象
CREATE USER user_a WITH PASSWORD 'pwd_a';
CREATE USER user_b WITH PASSWORD 'pwd_b';
CREATE USER user_c WITH PASSWORD 'pwd_c';

CREATE SCHEMA IF NOT EXISTS demo;

CREATE TABLE demo.sec_example (
    id   SERIAL PRIMARY KEY,
    info TEXT NOT NULL
);

INSERT INTO demo.sec_example (info) VALUES ('seed data 1'), ('seed data 2');

-- 1.2 授予与回收权限并观察登录行为
GRANT CONNECT ON DATABASE security_db TO user_a, user_b, user_c;
GRANT USAGE ON SCHEMA demo TO user_a, user_b, user_c;
GRANT SELECT ON demo.sec_example TO user_a;

-- 以 DBA 用户执行：切换至 A 账号测试可见对象
-- \c - user_a
-- SELECT * FROM demo.sec_example;

-- 撤销 A 的 SELECT 权限后再次尝试访问
REVOKE SELECT ON demo.sec_example FROM user_a;

-- ===========================
-- 2. 权限传递与回收链路实验
-- 设权限对象为 demo.sec_example 上的 SELECT
-- ===========================

-- 清理上一节的授权
REVOKE ALL PRIVILEGES ON demo.sec_example FROM user_a, user_b, user_c CASCADE;

-- 2.1 场景一：A -> B -> C
GRANT SELECT ON demo.sec_example TO user_a WITH GRANT OPTION;
SET ROLE user_a;
GRANT SELECT ON demo.sec_example TO user_b WITH GRANT OPTION;
SET ROLE user_b;
GRANT SELECT ON demo.sec_example TO user_c WITH GRANT OPTION;
-- 切换回 A
SET ROLE user_a;

-- 回收 B 的权限并检查 C
REVOKE SELECT ON demo.sec_example FROM user_b;
-- 有依赖，报错
REVOKE SELECT ON demo.sec_example FROM user_b CASCADE;
-- 预期：C 的权限被回收
SET ROLE user_c;
-- 尝试访问，预期失败
SELECT * FROM demo.sec_example;
-- 切换回 A 检查
SET ROLE user_a;
SELECT * FROM demo.sec_example;


-- 2.2 场景二：A 同时授权 B、C
GRANT SELECT ON demo.sec_example TO user_b WITH GRANT OPTION;
GRANT SELECT ON demo.sec_example TO user_c WITH GRANT OPTION;

-- B 再次转授给 C，形成多源权限
SET ROLE user_b;
GRANT SELECT ON demo.sec_example TO user_c WITH GRANT OPTION;
RESET ROLE;

-- 回收 B 的权限后，C 仍保留由 A 直接授予的权限
REVOKE SELECT ON demo.sec_example FROM user_b CASCADE;

-- 当回收 C 的权限时，可选择根据需要指明 RESTRICT 或 FROM GRANTOR 子句
REVOKE SELECT ON demo.sec_example FROM user_c;

-- 2.3 结果分析建议：
-- - 使用 \du+ 或查询 pg_default_acl / information_schema.role_table_grants 视图查看当前授权链。
-- - 注意 WITH GRANT OPTION 带来的依赖关系可由 REVOKE ... CASCADE 一次性清除。

-- ===========================
-- 3. TPCH 业务角色与测试用户
-- ===========================

-- 3.1 创建角色
CREATE ROLE supplier_mgr_role;
CREATE ROLE sales_mgr_role;
CREATE ROLE senior_analyst_role;

-- 3.2 授予角色所需权限
GRANT SELECT, UPDATE ON supplier TO supplier_mgr_role;
GRANT SELECT ON partsupp, part TO supplier_mgr_role;
GRANT SELECT ON nation, region TO supplier_mgr_role;

GRANT SELECT, UPDATE ON customer TO sales_mgr_role;
GRANT SELECT, INSERT, UPDATE ON orders TO sales_mgr_role;
GRANT SELECT ON lineitem TO sales_mgr_role;
GRANT SELECT ON nation TO sales_mgr_role;

DO $$
-- 声明一个名为 tbl 的变量，类型为 RECORD，用于接收循环里查询出的每一行。
DECLARE
    tbl RECORD;
-- 通过for循环拼接表省去对于每一张表写一个GRANT 
BEGIN
    FOR tbl IN SELECT tablename FROM pg_tables WHERE schemaname = 'public' LOOP
        EXECUTE format('GRANT SELECT ON TABLE public.%I TO senior_analyst_role', tbl.tablename);
    END LOOP;
END$$;

-- 3.3 创建测试用户并分配角色
CREATE USER supplier_mgr_user WITH PASSWORD 'supplier_pwd';
CREATE USER sales_mgr_user     WITH PASSWORD 'sales_pwd';
CREATE USER analyst_user       WITH PASSWORD 'analyst_pwd';

GRANT supplier_mgr_role TO supplier_mgr_user;
GRANT sales_mgr_role TO sales_mgr_user;
GRANT senior_analyst_role TO analyst_user;

-- 3.4 角色验证 SQL

-- Supplier Manager：拥有 supplier 更新、partsupp/part/nation/region 查询权限
SET ROLE supplier_mgr_user;
SELECT s_name FROM supplier LIMIT 1;
-- 预期：返回 1 行，说明具备 SELECT 能力（UPDATE 同时隐含 SELECT）

UPDATE supplier SET s_name = s_name WHERE s_suppkey = 1;
-- 预期：UPDATE 1（或 0，视是否存在 key=1），证明 UPDATE 权限生效

SELECT COUNT(*) FROM customer;
-- 预期：ERROR:  permission denied for table customer（未授予访问客户信息）

DELETE FROM supplier WHERE s_suppkey = 1;
-- 预期：ERROR:  permission denied for table supplier（未授予 DELETE）

RESET ROLE;

-- Sales Manager：允许维护订单/客户，禁写供应商
SET ROLE sales_mgr_user;

UPDATE orders SET o_totalprice = o_totalprice WHERE o_orderkey = 1;
-- 预期：UPDATE 1（或 0），确认对 orders 的 UPDATE 权限

BEGIN;
INSERT INTO orders (
    o_orderkey, o_custkey, o_orderstatus, o_totalprice, o_orderdate,
    o_orderpriority, o_clerk, o_shippriority, o_comment
) VALUES (
    900000,
    (SELECT c_custkey FROM customer LIMIT 1),
    'O',
    0.00,
    CURRENT_DATE,
    '5-LOW',
    'Clerk#000000001',
    0,
    'lab4 permission test'
);
-- 预期：INSERT 0 1，证明具备 INSERT 权限（事务稍后回滚避免脏数据）
ROLLBACK;
-- 预期：ROLLBACK 完成，测试插入不保留

UPDATE supplier SET s_name = s_name WHERE s_suppkey = 1;
-- 预期：ERROR:  permission denied for table supplier（销售经理无法维护供应商）

RESET ROLE;

-- Senior Analyst：只读全库
SET ROLE analyst_user;
SELECT COUNT(*) FROM nation;
-- 预期：成功返回计数，具备全库只读权限

UPDATE orders SET o_totalprice = o_totalprice WHERE o_orderkey = 1;
-- 预期：ERROR:  permission denied for table orders（无写入权限）

RESET ROLE;

-- 可选：查看角色授权链信息
SELECT
    grantee,
    grantor,
    table_schema,
    table_name,
    privilege_type,
    is_grantable
FROM information_schema.role_table_grants
WHERE grantee IN ('supplier_mgr_role', 'sales_mgr_role', 'senior_analyst_role')
  AND table_schema IN ('public', 'demo')
ORDER BY grantee, table_name, privilege_type;
-- 预期：输出三类角色的授权明细，可用于对照验证

-- ===========================
-- 4. 清理脚本
-- ===========================
-- DROP USER IF EXISTS user_alice, user_bob, user_cindy;
-- DROP USER IF EXISTS supplier_mgr_user, sales_mgr_user, analyst_user;
-- DROP ROLE IF EXISTS supplier_mgr_role, sales_mgr_role, senior_analyst_role;
-- DROP TABLE IF EXISTS demo.sec_example;
