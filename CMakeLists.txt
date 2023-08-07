# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (DNS_TEST1)
SET(CMAKE_C_COMPILER g++)
if(CMAKE_COMPILER_IS_GNUCXX)
   add_compile_options(-std=c++11)
   message(STATUS "optional:-std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)
link_libraries(ssl)
# 指定生成目标
add_executable(dns_TEST dns_main.cpp cache.cpp epoll.cpp message.cpp net.cpp)
target_link_libraries(ssl)
