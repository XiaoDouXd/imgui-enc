cmake_minimum_required(VERSION 3.24)
project(util)

# -------------------------------------------------- 一些设置
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# -------------------------------------------------- 链接文件
file(GLOB_RECURSE ALL_SOURCE "src/*.cpp" "src/*.c")
file(GLOB_RECURSE ALL_INCLUDE "include/*.h" "include/*.hpp")
add_library(${PROJECT_NAME} STATIC
    ${ALL_SOURCE}
    ${ALL_INCLUDE}
)
target_include_directories(${PROJECT_NAME} PUBLIC "./include")
