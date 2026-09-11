
# 查询优化方法讲解


### 查询优化概述

现代数据库在查询执行时会应用大量优化技术，包括但不限于：

- 查询重用技术
- 查询重写规则
- 查询算法优化技术
- 并行查询优化技术

作为数据库的“使用者”，我们不需要深入掌握每一种底层实现，但理解这些优化思想，可以帮助我们设计出**更合理、更高效的 SQL**。

## 二、查询优化原则（WHERE 子句）

### 1. 避免使用 `<>` 或 `!=`

在 `WHERE` 子句中应尽量避免使用 `<>` 或 `!=`，否则数据库引擎可能会放弃索引，进行全表扫描。

MySQL 仅在以下操作符中使用索引：

- `<`
- `<=`
- `=`
- `>`
- `>=`
- `BETWEEN`
- `IN`

---

## 三、索引优化要点

### 2. LIKE 查询与索引

可以使用索引的情况：

```sql
SELECT id FROM t WHERE col LIKE 'Mich%';

不能使用索引的情况：

SELECT id FROM t WHERE col LIKE '%ike';

原则：通配符 % 或 _ 不能出现在开头。

⸻

3. 避免在 WHERE 子句中使用 OR

OR 条件通常会导致索引失效，进而全表扫描。

不推荐写法：

SELECT id FROM t WHERE num = 10 OR num = 20;

推荐改写为：

SELECT id FROM t WHERE num = 10
UNION ALL
SELECT id FROM t WHERE num = 20;


⸻

4. 谨慎使用 IN 和 NOT IN

SELECT id FROM t WHERE num IN (1, 2, 3);

对于连续数值，推荐使用 BETWEEN：

SELECT id FROM t WHERE num BETWEEN 1 AND 3;


⸻

5. 避免对字段进行表达式运算

不推荐：

SELECT id FROM t WHERE num / 2 = 100;

推荐：

SELECT id FROM t WHERE num = 100 * 2;


⸻

6. 避免在 WHERE 子句中对字段使用函数

不推荐：

SELECT id FROM t WHERE SUBSTRING(name, 1, 3) = 'abc';
SELECT id FROM t WHERE DATEDIFF(day, createdate, '2005-11-30') = 0;

推荐：

SELECT id FROM t WHERE name LIKE 'abc%';
SELECT id FROM t 
WHERE createdate >= '2005-11-30'
  AND createdate <  '2005-12-01';


⸻

7. 复合索引使用原则
	•	使用复合索引时，必须包含索引的第一个字段
	•	条件字段顺序应尽量与索引顺序保持一致
	•	否则索引将不会被使用

⸻

8. 索引数量不宜过多
	•	索引可以提升 SELECT 性能
	•	但会降低 INSERT / UPDATE 性能
	•	一个表的索引数量 不建议超过 6 个
	•	需要定期检查是否存在“低频索引”

⸻

9. 谨慎更新聚簇索引列
	•	聚簇索引决定了表中数据的物理存储顺序
	•	更新聚簇索引字段会导致整表数据重排
	•	若该字段频繁更新，应慎重考虑是否建立为聚簇索引

⸻

四、其他优化建议

10. 使用数值类型优于字符类型
	•	数值比较只需一次
	•	字符比较需要逐字符比较
	•	数值字段查询和连接性能更好，存储开销更小

⸻

11. 使用 VARCHAR / NVARCHAR 代替 CHAR / NCHAR
	•	变长字段节省存储空间
	•	查询效率更高

⸻

12. 避免使用 SELECT *

不推荐：

SELECT * FROM t;

推荐：

SELECT id, name, age FROM t;

只返回必要字段，减少 IO 和网络开销。

⸻

五、总结
	1.	以上规则是通用经验法则，不同数据库优化器能力不同
	2.	写 SQL 时应培养良好的规范意识
	3.	善用 EXPLAIN、ANALYZE 工具进行实际分析
	4.	深层次优化还包括：
	•	硬件配置
	•	参数调优
	•	专业调优工具等