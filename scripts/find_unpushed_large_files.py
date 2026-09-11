#!/usr/bin/env python3
"""
列出本地在 Git 中存在但不在任何远端引用（remotes）中的 blob（文件），按大小降序排列。
用法示例：
  # 先更新远端 refs（可选，但建议）
  git fetch --all --prune
  # 列出前 20 个未推送的大文件
  python3 scripts/find_unpushed_large_files.py --top 20

脚本支持参数：
  --fetch            : 运行前执行 `git fetch --all --prune`（默认不 fetch）
  --top N            : 只显示前 N 项（默认 50；0 表示全部）
  --min-size BYTES   : 只显示大于等于指定字节数的项
  --human            : 以可读格式显示大小（KB/MB）
  --json             : 输出 JSON

注意：本脚本需要在 Git 仓库根目录或其子目录运行。
"""

import subprocess
import argparse
import sys
import os
import json


def run(cmd, input_text=None):
    return subprocess.run(cmd, input=input_text, text=True, capture_output=True, check=True)


def human_size(n):
    for unit in ['B','KB','MB','GB','TB']:
        if n < 1024.0:
            return f"{n:.1f}{unit}"
        n /= 1024.0
    return f"{n:.1f}PB"


def main():
    p = argparse.ArgumentParser(description='列出未推送到远端的 Git 大文件（按大小降序）。')
    p.add_argument('--fetch', action='store_true', help='先运行 git fetch --all --prune 更新远端 refs。')
    p.add_argument('--top', type=int, default=50, help='仅显示前 N 项，0 表示显示全部（默认 50）。')
    p.add_argument('--min-size', type=int, default=0, help='仅显示大小 >= 指定字节数的项。')
    p.add_argument('--human', action='store_true', help='以可读格式显示大小。')
    p.add_argument('--json', action='store_true', help='以 JSON 输出结果。')
    args = p.parse_args()

    # check git repo
    try:
        run(['git','rev-parse','--git-dir'])
    except subprocess.CalledProcessError:
        print('错误：当前目录不是一个 Git 仓库（或其子目录）。请在仓库根目录运行。', file=sys.stderr)
        sys.exit(2)

    if args.fetch:
        print('正在执行: git fetch --all --prune ...')
        try:
            subprocess.run(['git','fetch','--all','--prune'], check=True)
        except subprocess.CalledProcessError as e:
            print('git fetch 失败：', e, file=sys.stderr)

    # 获取本地可达但远端不可达的对象及其路径
    # rev-list --objects --all --not --remotes
    try:
        proc = run(['git','rev-list','--objects','--all','--not','--remotes'])
    except subprocess.CalledProcessError as e:
        print('执行 git rev-list 失败：', e, file=sys.stderr)
        sys.exit(2)

    lines = proc.stdout.splitlines()
    if not lines:
        print('未发现本地独有对象（本地与远端一致）。')
        return

    # map hash -> path (可能有重复路径指向同一 blob，保留第一个路径)
    hash_to_path = {}
    hashes = []
    for ln in lines:
        # format: <hash> <path>
        parts = ln.split(' ', 1)
        h = parts[0]
        path = parts[1] if len(parts) > 1 else ''
        if h not in hash_to_path and path:
            hash_to_path[h] = path
        if h not in hashes:
            hashes.append(h)

    if not hashes:
        print('未发现可列出的对象。')
        return

    # 批量查询对象类型与大小
    # 使用 git cat-file --batch-check="%(objectname) %(objecttype) %(objectsize)"
    try:
        p_proc = subprocess.Popen(['git','cat-file','--batch-check=%(objectname) %(objecttype) %(objectsize)'],
                                  stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        stdin_data = '\n'.join(hashes) + '\n'
        out, _ = p_proc.communicate(stdin_data)
    except Exception as e:
        print('调用 git cat-file 失败：', e, file=sys.stderr)
        sys.exit(2)

    entries = []
    for ln in out.splitlines():
        # format: <hash> <type> <size>
        sp = ln.split()
        if len(sp) < 3:
            continue
        h, typ, size_s = sp[0], sp[1], sp[2]
        try:
            size = int(size_s)
        except:
            size = 0
        if typ != 'blob':
            continue
        path = hash_to_path.get(h, '')
        entries.append((size, h, path))

    if not entries:
        print('未发现未推送的 blob 对象（或仅含非 blob 对象）。')
        return

    # 过滤 min-size
    entries = [e for e in entries if e[0] >= args.min_size]
    # 按大小降序
    entries.sort(key=lambda x: x[0], reverse=True)

    if args.top > 0:
        entries = entries[:args.top]

    # 输出
    if args.json:
        out_json = []
        for size,h,path in entries:
            out_json.append({'hash':h,'size':size,'path':path})
        print(json.dumps(out_json, ensure_ascii=False, indent=2))
        return

    # human-readable table
    print(f"共 {len(entries)} 项（按大小降序显示）：")
    for size,h,path in entries:
        size_str = human_size(size) if args.human else str(size)
        print(f"{size_str:>10}  {h}  {path}")


if __name__ == '__main__':
    main()
