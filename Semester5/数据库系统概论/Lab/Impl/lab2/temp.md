好的，没问题。这是一个非常标准的实验报告模板。我会根据我们之前的交流和你所完成的步骤，为你填充好这个模板的文字部分。你只需要在指定位置**插入你自己的截图**即可。

---

## **基于 PostgreSQL 的 TPC-H 数据库搭建与验证实验报告**

### **1. 实验环境**

*   **计算机配置**: `[请在此填写你的 Mac 型号，例如：MacBook Pro (14-inch, 2023), Apple M2 Pro]`
*   **操作系统**: `[请在此填写你的 macOS 版本，例如：macOS Sonoma 14.1]`
*   **数据库管理系统 (DBMS)**: PostgreSQL `[请在此填写你的版本号，例如：16.1]`
*   **安装与管理工具**: Homebrew, macOS 终端 (Terminal.app)
*   **数据库客户端**: `psql` 命令行工具
*   **实验数据**: TPC-H 数据集 (Scale Factor = 0.01)

### **2. 实验结果及分析**

#### **2.1 DBMS 安装过程**

本次实验通过 macOS 的包管理器 Homebrew 安装 PostgreSQL。主要步骤包括更新 Homebrew、执行安装命令以及启动数据库服务。安装过程顺利，验证了 PostgreSQL 服务的正常运行。

**(请在此处插入至少三张安装过程截图)**

*   **截图1：执行 `brew install postgresql` 命令的终端界面。**
    ![此处替换为你的截图1](https://via.placeholder.com/600x300.png?text=截图1：执行brew+install命令)
*   **截图2：安装成功后，Homebrew 显示的提示信息，包括如何启动服务。**
    ![此处替换为你的截图2](https://via.placeholder.com/600x300.png?text=截图2：安装成功提示)
*   **截图3：使用 `psql postgres` 成功连接到默认数据库的界面。**
    ![此处替换为你的截图3](https://via.placeholder.com/600x300.png?text=截图3：成功连接psql)

#### **2.2 执行DDL、DML语句**

成功连接到新建的 `tpch_db` 数据库后，首先执行了数据定义语言（DDL）语句，创建了 TPC-H 标准的 8 张数据表。随后，执行了数据操纵语言（DML）中的 `ALTER TABLE` 语句，为各表添加了主键和外键约束，以保证数据的实体完整性和引用完整性。

**(请在此处插入DDL、DML语句的运行截图)**

*   **截图4：执行 `CREATE TABLE` 语句，成功创建 8 张表的终端界面。可以附带 `\dt` 命令的输出来验证。**
    ![此处替换为你的截图4](https://via.placeholder.com/600x300.png?text=截图4：执行CREATE+TABLE和\dt命令)
*   **截图5：执行 `ALTER TABLE` 语句，为各表添加主键和外键约束，终端显示 `ALTER TABLE` 成功信息。**
    ![此处替换为你的截图5](https://via.placeholder.com/600x300.png?text=截图5：执行ALTER+TABLE添加约束)

#### **2.3 数据生成与导入**

使用 PostgreSQL 提供的 `\copy` 命令从预先生成的 `.tbl` 数据文件中批量导入数据。该命令效率高，能够快速将大规模数据加载到数据库中。通过指定 `|` 为分隔符，成功将所有数据文件导入到对应的表中。

**(请在此处插入数据导入的运行截图)**

*   **截图6：在 `psql` 中执行 `\copy` 命令导入数据的过程。截图应清晰显示至少几个表的导入命令及其返回的成功信息（如 `COPY 1500`）。**
    ![此处替换为你的截图6](https://via.placeholder.com/600x400.png?text=截图6：执行\copy命令导入数据)

#### **2.4 数据查询与验证**

数据导入完成后，进行了数据验证。首先，查询了各表的行数，结果与 TPC-H 在 SF=0.01 规模下的标准行数基本相符，证明数据导入完整。随后，执行了简单的单表查询和多表连接（JOIN）查询，均返回了预期的正确结果，表明数据库已搭建成功并可正常使用。

**(请在此处插入 SQL 执行截图)**

*   **截图7：执行查询各表行数的 SQL 语句及其结果。**
    ```sql
    SELECT 'region' AS table_name, count(*) FROM region UNION ALL
    SELECT 'nation', count(*) FROM nation UNION ALL
    SELECT 'part', count(*) FROM part UNION ALL
    SELECT 'supplier', count(*) FROM supplier UNION ALL
    SELECT 'partsupp', count(*) FROM partsupp UNION ALL
    SELECT 'customer', count(*) FROM customer UNION ALL
    SELECT 'orders', count(*) FROM orders UNION ALL
    SELECT 'lineitem', count(*) FROM lineitem;
    ```
    ![此处替换为你的截图7](https://via.placeholder.com/600x400.png?text=截图7：查询各表行数的结果)
*   **截图8：执行一条带 JOIN 的复杂查询语句及其返回的结果。**
    ```sql
    -- 示例查询：统计每个区域（region）有多少个供应商（supplier）
    SELECT
        r_name,
        count(s_suppkey) AS supplier_count
    FROM
        supplier
    JOIN
        nation ON s_nationkey = n_nationkey
    JOIN
        region ON n_regionkey = r_regionkey
    GROUP BY
        r_name
    ORDER BY
        supplier_count DESC;
    ```
    ![此处替换为你的截图8](https://via.placeholder.com/600x400.png?text=截图8：执行JOIN查询及其结果)

### **3. 实验总结**

本次实验系统地实践了数据库使用的核心流程。通过在 macOS 上安装配置 PostgreSQL，我掌握了 DBMS 的基本搭建方法。实验中，我亲手执行了 DDL 语句来构建 TPC-H 数据模型，并使用 `ALTER TABLE` 添加主外键约束，深刻理解了数据完整性在数据库设计中的重要性。使用 `\copy` 命令批量导入数据的过程，让我体验了真实场景下的数据加载操作。最后，通过编写并执行从简单到复杂的多表连接 SQL 查询，验证了数据库的正确性，并巩固了 SQL 查询技能。本次实验将理论与实践相结合，为后续更深入的数据库学习与应用打下了坚实的基础。