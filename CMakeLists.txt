# 设置 cmake 的最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(mprpc)

# 生成 Debug 版本，方便进行调试
set(CMAKE_BUILD_TYPE "Debug")

# 设置项目可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置项目的库文件和输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 向下递归
add_subdirectory(src)
add_subdirectory(example)