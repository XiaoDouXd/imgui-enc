cmake_minimum_required(VERSION 3.24)
get_filename_component(XD_PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" XD_PROJECT_NAME ${XD_PROJECT_NAME})
project(${XD_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# --------------------------------- 核心
add_subdirectory(util)
add_subdirectory(core)

# --------------------------------- 第三方库
add_subdirectory(3rd)

include(../Config.cmake)
file(GLOB_RECURSE ALL_SOURCE "core/src/*.cpp" "core/src/*.c" "core/src/*.h" "core/src/*.hpp" "core/include/*.h" "core/include/*.hpp")

add_library(${PROJECT_NAME} ${ALL_SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC ./core/include)
target_include_directories(${PROJECT_NAME} PRIVATE ./core/include/xdBase)
target_include_directories(${PROJECT_NAME} PRIVATE ./core/src)
target_include_directories(${PROJECT_NAME} PRIVATE ./core/src/core)

# 控制台仅在 Debug 模式下生成
if (${CMAKE_BUILD_TYPE} MATCHES ".*(D|d)(E|e)(B|b)(U|u)(G|g).*")
    message(STATUS "${PROJECT_NAME}::CMakeLists.txt::info: 将编译带有控制台的目标程序")
else()
    message(STATUS "${PROJECT_NAME}::CMakeLists.txt::info: 将编译隐藏控制台的目标程序")
    target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
endif()

# 链接 imgui
target_include_directories(${PROJECT_NAME} PUBLIC ./3rd/imgui/source/include)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)

# 链接 util
target_include_directories(${PROJECT_NAME} PUBLIC ./util/include)
target_link_libraries(${PROJECT_NAME} PUBLIC util)

# 链接 rc
target_include_directories(${PROJECT_NAME} PUBLIC ./core/rc/gen/include)
target_link_libraries(${PROJECT_NAME} PRIVATE rc)
