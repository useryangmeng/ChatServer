cmake_minimum_required(VERSION 3.0)
project(main)

#配置编译选项
set(CMAKE_CXX_FLASS ${CMAKE_CXX_FLASS} -g)

#配置头文件搜索路径
#include_directories()
#配置库文件搜索路径
#link_directories()

#设置需要编译的源文件列表
set(SRC_LIST ./muduotest.cpp)

#设置可执行文件最终存储路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#把指定目录下的所有文件放入变量
aux_source_directory(. SRC_LIST)

#生成可执行文件，由变量所定义的源文件编译而来
add_executable(server ${SRC_LIST})

#表示目标程序需要连接的库文件
target_link_libraries(server muduo_net muduo_base pthread)