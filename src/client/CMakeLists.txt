#定义一个变量包含该目录下所有源文件
aux_source_directory(. SRC_LIST)

add_executable(ChatClient ${SRC_LIST})

target_link_libraries(ChatClient pthread)