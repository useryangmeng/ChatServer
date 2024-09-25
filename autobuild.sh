#!/bin/bash
set -x

# 清空 build 目录
rm -rf "$(pwd)/build/*"

# 进入 build 目录
cd "$(pwd)/build"

# 运行 cmake 并构建
cmake .. && make
