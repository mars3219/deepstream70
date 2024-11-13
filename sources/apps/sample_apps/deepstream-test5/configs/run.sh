#!/bin/bash

# 커맨드라인 인자로 실행 횟수를 받음
n=$1

# n이 전달되지 않은 경우 기본값을 1로 설정
if [ -z "$n" ]; then
  echo "Enter the number of GUI windows to create."
  exit 1
fi

# 입력된 횟수만큼 반복
for ((i=1; i<=n; i++))
do
    echo "Running: $i times"
    ./deepstream-test5-app -c configs/test5_config_file_src_infer.yml -p 1 &
done

wait