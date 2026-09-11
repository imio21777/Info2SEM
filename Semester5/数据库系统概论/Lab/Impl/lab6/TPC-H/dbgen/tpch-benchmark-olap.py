'''
Author: ZhaoyangZhang
Date: 2024-11-24 19:09:56
LastEditors: Do not edit
LastEditTime: 2024-11-25 14:14:40
FilePath: /codes/TPC-H/dbgen/tpch-benchmark-olap.py
'''
import os
import time
import subprocess
from datetime import datetime

# 配置 MySQL 连接信息
MYSQL_USER = "root"
MYSQL_PASS = "12138"
MYSQL_DB = "tpch"
SQL_DIR = "./queries" 

timestamp = datetime.now().strftime('%Y%m%d%H%M%S')
COMPLETE_LOG_FILE = f"logs/tpch-benchmark-complete-{timestamp}.log"
SUMMARY_LOG_FILE = f"logs/tpch-benchmark-summary-{timestamp}.log"

sql_times = {}
total_query_time = 0
script_start_time = time.perf_counter()

with open(COMPLETE_LOG_FILE, "w") as complete_log, open(SUMMARY_LOG_FILE, "w") as summary_log:
    def log_complete(message):
        print(message)
        complete_log.write(message + "\n")

    def log_summary(message):
        summary_log.write(message + "\n")

    log_complete("=== Starting TPC-H Benchmark ===")
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
            cmd = f"mysql -u{MYSQL_USER} -p{MYSQL_PASS} -D{MYSQL_DB} < {sql_path}"
            result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

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
