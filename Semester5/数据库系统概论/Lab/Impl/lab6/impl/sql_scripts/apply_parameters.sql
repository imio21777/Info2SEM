-- 这是一个可以在当前会话中直接运行的脚本，用于测试参数调优效果。
-- 注意：shared_buffers 不能在会话级别修改，必须修改配置文件并重启数据库。
-- 这里我们主要调整可以在会话中立即生效的关键参数。

-- SET shared_buffers = '12GB';
-- negative change
-- actually use shared_buffers = 128MB


-- 1. work_mem: 增加每个操作（排序、哈希）可用的内存
SET work_mem = '256MB';

-- 2. random_page_cost: 调整优化器对随机 I/O 的成本估算
-- 默认是 4.0 (HDD)，调整为 1.1，鼓励使用索引
SET random_page_cost = 1.1;

-- 3. effective_cache_size: 告诉优化器系统有多少内存可用作文件缓存
-- 这不会实际分配内存，只是帮助优化器做决策
SET effective_cache_size = '12GB';

-- 4. effective_io_concurrency: 设置并发 I/O 能力 (SSD)
SET effective_io_concurrency = 200;

SET max_parallel_workers_per_gather = 16;

SET jit = off;

SHOW work_mem;
SHOW random_page_cost;
SHOW effective_cache_size;
SHOW effective_io_concurrency;
SHOW max_parallel_workers_per_gather;