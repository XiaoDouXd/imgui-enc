cmake_minimum_required(VERSION 3.25)
project(app)                           #------ 20230220 appBase 模块下沉

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# --------------------------------- 核心
add_subdirectory(base)                 #------ gui 模块和启动器
add_subdirectory(util)

# --------------------------------- 第三方库
add_subdirectory(3rd)