
### 2.1 单表查询

#### 1. 查询PART表中所有品牌名称以"BRAND"开头且类型包含"POLISHED"的商品，按商品尺寸降序排列。
```sql
SELECT *
FROM part
WHERE p_brand LIKE 'Brand%' AND p_type LIKE '%POLISHED%'
ORDER BY p_size DESC;
```

#### 2. 统计CUSTOMER表中每个市场区域(MKTSEGMENT)的客户数量。
```sql
SELECT c_mktsegment, COUNT(*) AS customer_count
FROM customer
GROUP BY c_mktsegment;
```

#### 3. 查询SUPPLIER表中，账户余额(ACCTBAL)平均值大于9000的国家(NATIONKEY)及其平均余额。
```sql
SELECT s_nationkey, AVG(s_acctbal) AS avg_balance
FROM supplier
GROUP BY s_nationkey
HAVING AVG(s_acctbal) > 9000;
```

#### 4. 查询ORDERS表中订单总金额(TOTALPRICE)最高的前10笔订单信息。
```sql
SELECT *
FROM orders
ORDER BY o_totalprice DESC
LIMIT 10;
```

#### 5. 查询LINEITEM表中每条订单明细的实际金额（扩展价格×(1-折扣)），并显示订单号、商品号和实际金额。
```sql
SELECT
    l_orderkey,
    l_partkey,
    l_extendedprice * (1 - l_discount) AS actual_price
FROM lineitem;
```

#### 6. 统计PART表中包装类型(CONTAINER)以"SM"开头的各类包装的商品数量。
```sql
SELECT p_container, COUNT(*) AS p_counts
FROM part
WHERE p_container LIKE 'SM%'
GROUP BY p_container;
```

#### 7. 查询LINEITEM表中，发货模式(SHIPMODE)为"AIR"或"TRUCK"，且平均数量大于25的发货模式。
```sql
SELECT l_shipmode, AVG(l_quantity) AS avg_quantity
FROM lineitem
WHERE l_shipmode IN ('AIR', 'TRUCK')
GROUP BY l_shipmode
HAVING AVG(l_quantity) > 25;
```

#### 8. 查询CUSTOMER表，计算每个客户的折扣余额（账户余额×0.9），并按折扣余额降序排列。
```sql
SELECT
    c_custkey,
    c_name,
    c_acctbal,
    c_acctbal * 0.9 AS discounted_balance
FROM customer
ORDER BY discounted_balance DESC;
```

#### 9. 查询SUPPLIER表中，电话号码以"15"开头的供应商，按国家分组后，筛选出供应商数量超过5个的国家。
```sql
SELECT s_nationkey, COUNT(*) AS supplier_count
FROM supplier
WHERE s_phone LIKE '15%'
GROUP BY s_nationkey
HAVING COUNT(*) > 5;
```

#### 10. 查询ORDERS表中订单优先级(PRIORITY)包含"URGENT"的订单，按订单日期升序、总金额降序排列，显示前15条。
```sql
SELECT *
FROM orders
WHERE o_orderpriority LIKE '%URGENT%'
ORDER BY o_orderdate ASC, o_totalprice DESC
LIMIT 15;
```

#### 11. 查询PART表，找出品牌名称第二个字符为大写字母"R"，且制造商名称以"Manufacturer"开头的商品，计算每个尺寸组的平均零售价。
```sql
SELECT p_size, AVG(p_retailprice) AS avg_retailprice
FROM part
WHERE p_mfgr LIKE 'Manufacturer%' AND p_brand LIKE '_R%'
-- AND p_brand LIKE '_R%' 是无结果的
GROUP BY p_size
ORDER BY p_size;
-- 加上ORDER BY便于查看
```

#### 12. 查询LINEITEM表，统计1995-1996年间每个季度的订单明细数量，只显示数量超过1000的季度。
```sql
SELECT
    EXTRACT(YEAR FROM l_shipdate) AS year,
    EXTRACT(QUARTER FROM l_shipdate) AS quarter,
    COUNT(*) AS item_count
FROM lineitem
WHERE l_shipdate BETWEEN DATE '1995-01-01' AND DATE '1996-12-31'
GROUP BY year, quarter
HAVING COUNT(*) > 1000
ORDER BY year, quarter;
```

### 2.2 多表查询

#### 1.	查询每个国家(NATION)的客户数量，显示国家名称和客户数量。
```sql
SELECT n_name, COUNT(c.c_custkey) AS customer_count
FROM nation n
JOIN customer c ON n.n_nationkey = c.c_nationkey
GROUP BY n_name
ORDER BY customer_count DESC;
```

#### 2. 查询每个地区(REGION)的供应商数量，显示地区名称和供应商数量
```sql
SELECT r.r_name AS region_name, COUNT(s.s_suppkey) AS supplier_count
FROM supplier s
JOIN nation n ON n.n_nationkey = s.s_nationkey
JOIN region r ON r.r_regionkey = n.n_regionkey
GROUP BY r.r_name;
```

#### 3. 查询单个订单总金额超过10000的客户名称及其订单总金额。
```sql
SELECT c.c_name AS customer_name, o.o_totalprice AS total_amount
FROM customer c
JOIN orders o ON c.c_custkey = o.o_custkey
WHERE o.o_totalprice > 10000
ORDER BY o.o_totalprice DESC;
```
> 这里其实表述不是很清楚，订单总金额指的是单个订单还是这个客户的所有订单呢，如果是所有订单就要像下面这样写了
> 不过我们暂且还是理解成单个
> ``` sql
> SELECT c.c_name AS customer_name, SUM(o.o_totalprice) AS total_amount
> FROM customer c
> JOIN orders o ON c.c_custkey = o.o_custkey
> GROUP BY c.c_name
> HAVING SUM(o.o_totalprice) > 10000
> ORDER BY SUM(o.o_totalprice) DESC;
> ```

#### 4. 查询至少有一个订单的客户信息。
```sql
SELECT c.*
FROM customer c
WHERE EXISTS (
    SELECT 1 
    FROM orders o 
    WHERE o.o_custkey = c.c_custkey
);

-- Use join
SELECT DISTINCT c.*
FROM customer c
JOIN orders o ON c.c_custkey = o.o_custkey
ORDER BY c.c_custkey;
```
> 直接思路是直接从customer表中选然后一个个判断有订单还是没有
> 当然也可以直接把customer表和orders表 JOIN 起来，会自动过滤掉没有订单的
> 不过这里每个客户可能有多个订单，因此加一个 DISTINCT 去重

#### 5. 查询每个国家的平均客户账户余额，结果显示国家名称和平均客户账户余额。
```sql
SELECT n.n_name AS nation_name, AVG(c.c_acctbal) AS avg_acctbal
FROM customer c
JOIN nation n ON c.c_nationkey = n.n_nationkey
GROUP BY n.n_name;
```

#### 6. 查询没有下过任何订单的客户信息。
```sql
SELECT c.*
FROM customer c
WHERE NOT EXISTS (
    SELECT 1 
    FROM orders o 
    WHERE o.o_custkey = c.c_custkey
)
ORDER BY c.c_custkey;

-- Use join
SELECT c.*
FROM customer c
LEFT JOIN orders o ON c.c_custkey = o.o_custkey
WHERE o.o_orderkey IS NULL
ORDER BY c.c_custkey;
```

#### 7. 查询在同一国家的客户对（不重复组合）。
```sql
SELECT c1.c_name AS customer1, c2.c_name AS customer2, n.n_name AS nation_name
FROM customer c1
JOIN customer c2 ON c1.c_nationkey = c2.c_nationkey AND c1.c_custkey < c2.c_custkey
JOIN nation n ON c1.c_nationkey = n.n_nationkey;
```

#### 8. 查询订单总金额超过该客户平均订单金额的订单信息。
```sql
SELECT o.*
FROM orders o
JOIN (
    SELECT o_custkey, AVG(o_totalprice) AS avg_order_amount
    FROM orders
    GROUP BY o_custkey
) cust_avg ON o.o_custkey = cust_avg.o_custkey
WHERE o.o_totalprice > cust_avg.avg_order_amount;
```
> 先用子查询查出每个用户的平均订单金额
> 然后拼到每个用户上再select

#### 9. 查询供应了所有类型商品的供应商信息。
```sql
SELECT s.*
FROM supplier s
WHERE NOT EXISTS (
    -- 找出供应商s没有供应的零件
    SELECT p.p_type
    FROM part p
    -- 不存在找不到该记录
    WHERE NOT EXISTS (
        -- 找出 partsupp 供应关系表中，属于供应商 s，对应零件 p 的那条供应记录。
        -- 即确认供应商 s 是否供应了零件 p
        SELECT 1
        FROM partsupp ps
        WHERE ps.ps_suppkey = s.s_suppkey AND ps.ps_partkey = p.p_partkey
    )
);

-- 使用HAVING
SELECT s.*
-- 直接全拼到供应商表上
-- 看供应商表提供的独特商品p个数和总表个数一样不一样就可以了
FROM supplier s
JOIN partsupp ps ON s.s_suppkey = ps.ps_suppkey
JOIN part p ON ps.ps_partkey = p.p_partkey
GROUP BY s.s_suppkey, s.s_name, s.s_address, s.s_nationkey, s.s_phone, s.s_acctbal, s.s_comment
HAVING COUNT(DISTINCT p.p_type) = (SELECT COUNT(DISTINCT p_type) FROM part);
```
> 两个 NOT EXIST 这个逻辑确实有点绕，难想出来...
> 具体解释看注释
> 值得一提的是这里没有供应商供应了所有商品。

#### 10. 查询总价大于所处月份平均总价的订单。
```sql
-- Solution1:
-- 遍历 orders 表的每一行，对于每一行：
-- 查看该订单的年份和月份。
-- 找到所有与它同年同月的其他订单（即它所在的分区）。
-- 计算这个分区内所有订单的 o_totalprice 的平均值。
-- 将这个算出的平均值作为新的一列 avg_monthly_price 附加到当前行上。
WITH MonthlyAvg AS (
    SELECT *,
-- 2. 添加一个计算出的新列,关键字 OVER 表明这是一个窗口函数，不会像 GROUP BY 那样把多行压缩成一行，而是保持原有行数不变，为每一行计算一个附加值。
    AVG(o_totalprice) OVER (PARTITION BY EXTRACT(YEAR FROM o_orderdate), EXTRACT(MONTH FROM o_orderdate)) AS avg_monthly_price
    FROM orders
)
SELECT MonthlyAvg.o_orderkey, o_totalprice, avg_monthly_price 
FROM MonthlyAvg
WHERE o_totalprice > avg_monthly_price
ORDER BY o_orderkey;

-- Solution2:
SELECT o.*
FROM orders o
JOIN (
  -- 求每个年/月的平均总价
    SELECT 
        EXTRACT(YEAR FROM o_orderdate) AS order_year,
        EXTRACT(MONTH FROM o_orderdate) AS order_month,
        AVG(o_totalprice) AS monthly_avg
    FROM orders
    GROUP BY EXTRACT(YEAR FROM o_orderdate), EXTRACT(MONTH FROM o_orderdate)
) monthly_avg ON EXTRACT(YEAR FROM o.o_orderdate) = monthly_avg.order_year 
               AND EXTRACT(MONTH FROM o.o_orderdate) = monthly_avg.order_month
WHERE o.o_totalprice > monthly_avg.monthly_avg;
```
> AI直接上了WITH这个没学过的语法...
> 第二种虽然看着烦但是容易理解一点
>
> ##### 题目1： 查找这样的customer，他购买了CENTC(customer_id)买过的所有商品。结果显示customer_id，并升序排列

``` sql
SELECT DISTINCT c.customer_id
FROM northwind.orders c                    -- 外层：枚举所有客户的订单（每行是一个订单，但后面 DISTINCT 会按客户去重）
-- 条件：不存在这样一件“CENTC 买过但这个客户没买过”的商品
WHERE NOT EXISTS (                          
    SELECT 1
    FROM northwind.order_details od_centc   -- 内层①：枚举 CENTC 买过的每一条明细商品
    JOIN northwind.orders o_centc
      ON od_centc.order_id = o_centc.order_id
    WHERE o_centc.customer_id = 'CENTC'     -- 锁定客户：CENTC
      AND NOT EXISTS (                      -- 对于 CENTC 的这件商品，再检查外层客户 c 是否也买过
          SELECT 1
          FROM northwind.order_details od_other
          JOIN northwind.orders o_other
            ON od_other.order_id = o_other.order_id
          WHERE o_other.customer_id = c.customer_id      -- 指定：当前被检查的这个客户 c
            AND od_other.product_id = od_centc.product_id -- 必须买过“CENTC 当前这一条商品”
      )
      -- 如果能找到一条 CENTC 买过的商品（od_centc），
      -- 但外层客户 c 在自己的所有订单里都“找不到”同样的 product_id，
      -- 那么 NOT EXISTS 为 true，就说明 c 少买了一样 CENTC 买过的产品。
      -- 外层的 WHERE NOT EXISTS(...) 就会被破坏，c 就被排除掉。
)
-- 如果走到这里，说明：对于 CENTC 买过的每一件商品，
-- 外层客户 c 都能找到一件“相同 product_id 的商品”——也就是 c 至少买了 CENTC 买过的所有品种
ORDER BY c.customer_id ASC;
```

##### 题目2： 查找在1996-7-1（含）之后下单数量大于等于10笔的顾客，结果显示customer_id, company_name，订单数量。按订单数量降序排列，若订单数量相同则按customer_id升序排列

``` sql
SELECT c.customer_id,c.company_name,COUNT(o.order_id) AS 订单数量
FROM northwind.customers c
JOIN northwind.orders o
ON c.customer_id = o.customer_id
WHERE o.order_date >= DATE '1996-07-01'
GROUP BY c.customer_id, c.company_name
HAVING COUNT(o.order_id) >= 10
ORDER BY 订单数量 DESC, c.customer_id ASC;
```

##### 题目3： 计算每个类别的平均单价（average_price），结果显示category_name，average_price，并按类别名称（category_name）升序排列

``` sql
SELECT c.category_name, AVG(p.unit_price) as average_price
FROM northwind.categories c
JOIN northwind.products p
ON p.category_id = c.category_id
GROUP BY c.category_name
ORDER BY c.category_name ASC
```

##### 题目4：查询每位客户的最大订单运费freight以及对应的订单编号order_id，订单日期order_date，客户编号customer_id，客户名称company_name，并按客户编号customer_id升序排列

``` sql
SELECT 
    o.customer_id,
    c.company_name,
    o.order_id,
    o.order_date,
    o.freight
FROM northwind.orders o
JOIN (
    SELECT customer_id, MAX(freight) AS max_freight
    FROM northwind.orders
    GROUP BY customer_id
) mf
    ON o.customer_id = mf.customer_id
   AND o.freight = mf.max_freight
JOIN northwind.customers c
    ON o.customer_id = c.customer_id
ORDER BY o.customer_id ASC;
```

##### 题目5： 查询每个员工处理的不同客户的数量，显示每个员工的first_name，last_name，以及处理不同客户的数量（customer_num），只显示那些处理了超过10个不同客户的员工，结果按处理不同客户的数量降序排列 a

``` sql
SELECT e.first_name,e.last_name,COUNT(DISTINCT o.customer_id) AS customer_num
FROM northwind.employees e
JOIN northwind.orders o
ON o.employee_id = e.employee_id
GROUP BY e.first_name,e.last_name
HAVING COUNT(DISTINCT o.customer_id)>10
ORDER BY customer_num DESC;
```

##### 1996年7月份的销冠是哪个employee
``` sql
SELECT
    e.employee_id,
    e.last_name,
    e.first_name,
    SUM(od.quantity * od.unit_price * (1 - od.discount)) AS total_sales
FROM northwind.orders AS o
JOIN northwind.order_details AS od ON o.order_id = od.order_id
JOIN northwind.employees AS e ON o.employee_id = e.employee_id
WHERE o.order_date >= '1996-07-01' AND o.order_date <  '1996-08-01'
GROUP BY
    e.employee_id,
    e.last_name,
    e.first_name
ORDER BY total_sales DESC
LIMIT 1;
```

##### 哪一个供应商供应的某一件物品的单价在50美元以下占了20%以上的有哪些
``` sql
-- 单价<50美元的产品占比>20%的供应商
SELECT
    s.supplier_id,
    s.company_name,
    COUNT(*) AS total_products,
    SUM(CASE WHEN p.unit_price < 50 THEN 1 ELSE 0 END) AS products_less_than_50,
    SUM(CASE WHEN p.unit_price < 50 THEN 1 ELSE 0 END) / COUNT(*) AS pct_less_than_50
FROM northwind.suppliers AS s
JOIN northwind.products AS p
    ON s.supplier_id = p.supplier_id
GROUP BY
    s.supplier_id,
    s.company_name
HAVING
    -- 为了避免整数除法，这里乘1.0
    (SUM(CASE WHEN p.unit_price < 50 THEN 1 ELSE 0 END) * 1.0) / COUNT(*) > 0.2;
```

##### 哪一个国家的雇员和顾客的人数比最大
``` sql
SELECT
    t.country,
    t.emp_cnt,
    t.cust_cnt,
    (t.emp_cnt * 1.0 / t.cust_cnt) AS emp_to_cust_ratio
FROM (
    -- 把“各国员工数”和“各国客户数”合在一起
    SELECT
        COALESCE(e.country, c.country) AS country,
        COALESCE(e.emp_cnt, 0) AS emp_cnt,
        COALESCE(c.cust_cnt, 0) AS cust_cnt
    FROM (
        SELECT country, COUNT(*) AS emp_cnt
        FROM northwind.employees
        GROUP BY country
    ) AS e
    FULL JOIN (
        SELECT country, COUNT(*) AS cust_cnt
        FROM northwind.customers
        GROUP BY country
    ) AS c
      ON e.country = c.country
) AS t
WHERE t.cust_cnt > 0                      -- 避免除以0
ORDER BY (t.emp_cnt * 1.0 / t.cust_cnt) DESC
LIMIT 1;

SELECT
    t.country,
    SUM(t.emp_cnt)  AS emp_cnt,
    SUM(t.cust_cnt) AS cust_cnt,
    (SUM(t.emp_cnt) * 1.0 / SUM(t.cust_cnt)) AS emp_to_cust_ratio
FROM (
    -- 员工：有员工数，没有客户数
    SELECT
        country,
        COUNT(*) AS emp_cnt,
        0        AS cust_cnt
    FROM northwind.employees
    GROUP BY country

    UNION ALL

    -- 客户：有客户数，没有员工数
    SELECT
        country,
        0        AS emp_cnt,
        COUNT(*) AS cust_cnt
    FROM northwind.customers
    GROUP BY country
) AS t
GROUP BY t.country
HAVING SUM(t.cust_cnt) > 0              -- 避免除以 0，没有客户的国家不要
ORDER BY (SUM(t.emp_cnt) * 1.0 / SUM(t.cust_cnt)) DESC
LIMIT 1;
```

##### 哪一个顾客付款最多，付款的计算方式是物品件数乘以物品单价乘以一减去折扣数
``` sql
SELECT
    c.customer_id,
    c.company_name,
    SUM(od.quantity * od.unit_price * (1 - od.discount)) AS total_payment
FROM northwind.customers AS c
JOIN northwind.orders AS o
    ON c.customer_id = o.customer_id
JOIN northwind.order_details AS od
    ON o.order_id = od.order_id
GROUP BY
    c.customer_id,
    c.company_name
ORDER BY total_payment DESC
LIMIT 1;
```