# !/bin/bash

# 打开命令行
set -e

# 清空 build 下的内容
rm -rf `pwd`/build/*

# 执行 cmake 构建文件
cd `pwd`/build &&
    cmake .. &&
    make

cd ..

cp -r `pwd`/src/include `pwd`/lib