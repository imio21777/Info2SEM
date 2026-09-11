-- 实验四 触发器实验 SQL 脚本

-- ================
-- 1. UPDATE 触发器
-- ================
CREATE OR REPLACE FUNCTION public.trg_lineitem_after_update()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
DECLARE
    old_amount NUMERIC := COALESCE(OLD.l_extendedprice, 0) * (1 - COALESCE(OLD.l_discount, 0));
    new_amount NUMERIC := COALESCE(NEW.l_extendedprice, 0) * (1 - COALESCE(NEW.l_discount, 0));
BEGIN
    UPDATE orders
    SET o_totalprice = COALESCE(o_totalprice, 0) + (new_amount - old_amount)
    WHERE o_orderkey = NEW.l_orderkey;

    RETURN NEW;
END;
$$;

-- CREATE OR REPLACE FUNCTION public.trg_lineitem_after_update()
-- RETURNS TRIGGER
-- LANGUAGE plpgsql
-- AS $$
-- DECLARE
--     old_amount NUMERIC := COALESCE(OLD.l_extendedprice, 0) * (1 - COALESCE(OLD.l_discount, 0));
--     new_amount NUMERIC := COALESCE(NEW.l_extendedprice, 0) * (1 - COALESCE(NEW.l_discount, 0));
-- BEGIN
--     IF NEW.l_orderkey IS DISTINCT FROM OLD.l_orderkey THEN
--         -- 明细转移到其他订单：先减旧单，再加新单
--         UPDATE orders
--         SET o_totalprice = COALESCE(o_totalprice, 0) - old_amount
--         WHERE o_orderkey = OLD.l_orderkey;

--         UPDATE orders
--         SET o_totalprice = COALESCE(o_totalprice, 0) + new_amount
--         WHERE o_orderkey = NEW.l_orderkey;
--     ELSE
--         -- 同一订单，仅需调整差值
--         UPDATE orders
--         SET o_totalprice = COALESCE(o_totalprice, 0) + (new_amount - old_amount)
--         WHERE o_orderkey = NEW.l_orderkey;
--     END IF;

--     RETURN NEW;
-- END;
-- $$;

DROP TRIGGER IF EXISTS lineitem_update_totalprice ON lineitem;

CREATE TRIGGER lineitem_update_totalprice
AFTER UPDATE OF l_extendedprice, l_discount, l_orderkey ON lineitem
FOR EACH ROW
EXECUTE FUNCTION public.trg_lineitem_after_update();

-- ================
-- 2. INSERT 触发器
-- ================


CREATE OR REPLACE FUNCTION public.trg_lineitem_after_insert()
RETURNS TRIGGER
LANGUAGE plpgsql
AS $$
DECLARE
    new_amount NUMERIC := COALESCE(NEW.l_extendedprice, 0) * (1 - COALESCE(NEW.l_discount, 0));
BEGIN
    UPDATE orders
    SET o_totalprice = COALESCE(o_totalprice, 0) + new_amount
    WHERE o_orderkey = NEW.l_orderkey;
    RETURN NEW;
END;
$$;

DROP TRIGGER IF EXISTS lineitem_insert_totalprice ON lineitem;

CREATE TRIGGER lineitem_insert_totalprice
AFTER INSERT ON lineitem
FOR EACH ROW
EXECUTE FUNCTION public.trg_lineitem_after_insert();

-- ================
-- 3. 验证建议
-- ================

-- 1) UPDATE 测试：折扣从 0.05 改到 0.10，预期订单总价减少 5% * l_extendedprice
BEGIN;

SELECT o_totalprice AS total_before
INTO TEMP TABLE tmp_order_total_before
FROM orders
WHERE o_orderkey = 1;

UPDATE lineitem
SET l_discount = 0.10
WHERE l_orderkey = 1
  AND l_linenumber = 1;
-- 预期：UPDATE 1

SELECT o_totalprice AS total_after
FROM orders
WHERE o_orderkey = 1;
-- 预期：total_after = total_before - (OLD.l_extendedprice * (0.10 - 0.05))

select l_extendedprice from lineitem
where l_orderkey = 1
  and l_linenumber = 1;

DROP TABLE tmp_order_total_before;
ROLLBACK;

-- 2) INSERT 测试：新增一条明细，预期订单总价增加 新明细金额
BEGIN;

SELECT o_totalprice
INTO TEMP TABLE tmp_order_total_insert
FROM orders
WHERE o_orderkey = 2;

INSERT INTO lineitem (
    l_orderkey, l_partkey, l_suppkey, l_linenumber,
    l_quantity, l_extendedprice, l_discount, l_tax,
    l_returnflag, l_linestatus, l_shipdate, l_commitdate,
    l_receiptdate, l_shipinstruct, l_shipmode, l_comment
)
VALUES (
    2,
    (SELECT p_partkey FROM part LIMIT 1),
    (SELECT s_suppkey FROM supplier LIMIT 1),
    999,        -- 确保不与现有行冲突
    1,
    123.45,
    0.05,
    0,
    'N',
    'O',
    CURRENT_DATE,
    CURRENT_DATE,
    CURRENT_DATE,
    'DELIVER IN PERSON',
    'AIR',
    'lab4 trigger test'
);
-- 预期：INSERT 0 1

SELECT o_totalprice
FROM orders
WHERE o_orderkey = 2;
-- 预期：新 totalprice = 原 totalprice + 123.45 * (1 - 0.05)

ROLLBACK;

-- 3) 在测试完成后，删除触发器：
--    DROP TRIGGER lineitem_update_totalprice ON lineitem;
--    DROP TRIGGER lineitem_insert_totalprice ON lineitem;
--    DROP FUNCTION public.trg_lineitem_after_update();
--    DROP FUNCTION public.trg_lineitem_after_insert();
