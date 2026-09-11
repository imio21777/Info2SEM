import os
import time
import subprocess
from datetime import datetime

# --- 配置 PostgreSQL 连接信息 ---
PG_USER = "leap"      # 默认通常是 postgres
PG_PASS = "5432"         # 你的密码
PG_DB = "tpchdb"            # 数据库名
PG_HOST = "localhost"     # 数据库地址
PG_PORT = "5432"          # 默认端口 5432
SQL_DIR = "./queries" 

# 创建日志目录（如果不存在）
os.makedirs("logs", exist_ok=True)

timestamp = datetime.now().strftime('%Y%m%d%H%M%S')
COMPLETE_LOG_FILE = f"logs/tpch-benchmark-complete-{timestamp}.log"
SUMMARY_LOG_FILE = f"logs/tpch-benchmark-summary-{timestamp}.log"

sql_times = {}
total_query_time = 0
script_start_time = time.perf_counter()

# 设置 PostgreSQL 密码环境变量，避免交互式输入
env = os.environ.copy()
env["PGPASSWORD"] = PG_PASS

with open(COMPLETE_LOG_FILE, "w") as complete_log, open(SUMMARY_LOG_FILE, "w") as summary_log:
    def log_complete(message):
        print(message)
        complete_log.write(message + "\n")

    def log_summary(message):
        summary_log.write(message + "\n")

    log_complete("=== Starting TPC-H Benchmark (PostgreSQL) ===")
    log_complete(f"Complete Log File: {COMPLETE_LOG_FILE}\n")
    log_complete(f"Summary Log File: {SUMMARY_LOG_FILE}\n")
    log_complete(f"Start Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")

    # 遍历 SQL 文件 (d1.sql 到 d22.sql)
    for i in range(1, 23):
        sql_file = f"d{i}.sql"
        sql_path = os.path.join(SQL_DIR, sql_file)

        if not os.path.exists(sql_path):
            log_complete(f"Query {sql_file} skipped: File not found.")
            continue

        log_complete(f"Query {sql_file} starting...")
        query_start_time = time.perf_counter()

        try:
            # 修改后的 psql 命令：
            # -U: 用户名, -d: 数据库, -h: 主机, -p: 端口, -f: 执行指定文件
            # -q: 静默模式（可选）, -t: 只打印行（可选）
            cmd = [
                "psql", 
                "-U", PG_USER, 
                "-d", PG_DB, 
                "-h", PG_HOST, 
                "-p", PG_PORT, 
                "-f", sql_path
            ]
            
            # 使用 subprocess.run 执行，并传入包含 PGPASSWORD 的环境变量
            result = subprocess.run(
                cmd, 
                env=env, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE, 
                text=True
            )

            if result.returncode != 0:
                log_complete(f"Query {sql_file} failed with error:\n{result.stderr}")
            else:
                log_complete(f"Query {sql_file} output:\n{result.stdout}")

        except Exception as e:
            log_complete(f"Query {sql_file} encountered an exception: {e}")

        query_end_time = time.perf_counter()
        query_time = query_end_time - query_start_time
        sql_times[sql_file] = query_time
        total_query_time += query_time
        log_complete(f"Query {sql_file} ended! Time: {query_time:.6f} seconds\n")

    script_end_time = time.perf_counter()
    script_total_time = script_end_time - script_start_time

    log_complete("=== SQL Execution Summary ===")
    log_summary("=== SQL Execution Summary ===")

    for sql_file, elapsed_time in sql_times.items():
        summary_line = f"Query {sql_file:10}: {elapsed_time:.6f} seconds"
        log_complete(summary_line)
        log_summary(summary_line)

    total_query_time_line = f"\nTotal Execution Time for Queries: {total_query_time:.6f} seconds"
    script_total_time_line = f"Script Total Time (including overhead): {script_total_time:.6f} seconds"

    log_complete(total_query_time_line)
    log_complete(script_total_time_line)

    log_summary(total_query_time_line)
    log_summary(script_total_time_line)

    log_complete("=== Benchmark Completed ===")
    log_summary("=== Benchmark Completed ===")