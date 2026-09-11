##### 题目1： 查找这样的customer，他购买了CENTC(customer_id)买过的所有商品。结果显示customer_id，并升序排列

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