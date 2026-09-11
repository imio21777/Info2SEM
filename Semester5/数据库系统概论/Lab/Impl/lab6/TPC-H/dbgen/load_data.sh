#!/bin/bash
###
# @Author: ZhaoyangZhang
# @Date: 2024-11-25
# @Usage: ./load_data.sh <filename> <line_per_file>
###

if [ $# -lt 1 ]; then
    echo "Usage: $0 <filename> [line_per_file]"
    exit 1
fi

#  `partsupp`、`orders` or `lineitem` 
filename=$1

# set default line_per_file to 1000000 if not provided
line=${2:-1000000}

totalline=$(cat "$filename.tbl" | wc -l)
echo "Total lines: $totalline"

a=$((totalline / line))
b=$((totalline % line))
if (( b == 0 )); then
    filenum=$a
else
    filenum=$((a + 1))
fi
echo "Number of split files: $filenum"

split_dir="./split_files"
mkdir -p "$split_dir"

current_dir=$(pwd)
echo "Current directory: $current_dir"

# disable foreign key and unique checks
mysql -uroot -p'12138' -Dtpch -e "
SET GLOBAL foreign_key_checks = 0;
SET GLOBAL unique_checks = 0;"
echo "Foreign key and unique checks disabled."

# disable keys
mysql -uroot -p'12138' -Dtpch -e "
ALTER TABLE $filename DISABLE KEYS;"
echo "Indexes disabled for $filename."

total_start_time=$(date +%s)
declare -a times  # 数组记录每个小文件的加载时间
declare -a failed_files  # 数组记录失败文件

for ((i=1; i<=filenum; i++)); do
    min=$(((i - 1) * line + 1))
    max=$((i * line))

    split_file="$split_dir/$filename.tbl.$i"
    sed -n "${min},${max}p" "./$filename.tbl" > "$split_file"
    echo "File $split_file created."

    absolute_path="$current_dir/$split_file"

    start_time=$(date +%s)

    mysql -uroot -p'12138' -Dtpch -e \
        "LOAD DATA INFILE '$absolute_path' INTO TABLE $filename FIELDS TERMINATED BY '|';" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "File $absolute_path imported into MySQL."
    else
        echo "Error importing $absolute_path into MySQL. Adding to failed files."
        failed_files+=("$absolute_path")
    fi

    end_time=$(date +%s)
    elapsed_time=$((end_time - start_time))
    times[$i]=$elapsed_time 
    echo "Import time for $split_file: ${elapsed_time}s"
done

# restore indexes and foreign key/unique checks
mysql -uroot -p'12138' -Dtpch -e "
ALTER TABLE $filename ENABLE KEYS;
SET GLOBAL foreign_key_checks = 1;
SET GLOBAL unique_checks = 1;"
echo "Indexes re-enabled and foreign key/unique checks restored."

total_end_time=$(date +%s)
total_elapsed_time=$((total_end_time - total_start_time))

echo "=== Import Times Summary ==="
for ((i=1; i<=filenum; i++)); do
    echo "File $split_dir/$filename.tbl.$i: ${times[$i]}s"
done
echo "Total import time: ${total_elapsed_time}s"

if [ ${#failed_files[@]} -ne 0 ]; then
    echo "=== Failed Files ==="
    for file in "${failed_files[@]}"; do
        echo "$file"
    done
else
    echo "All files imported successfully!"
fi
