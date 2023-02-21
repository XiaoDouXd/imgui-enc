cmake_minimum_required(VERSION 3.25)
project(base)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include(../Config.cmake)
add_subdirectory(rc)
file(GLOB_RECURSE ALL_SOURCE "src/*.cpp" "src/*.c" "src/*.h" "src/*.hpp" "include/*.h" "include/*.hpp")

add_library(${PROJECT_NAME} ${ALL_SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC ./include)
target_include_directories(${PROJECT_NAME} PRIVATE ./src)
target_include_directories(${PROJECT_NAME} PRIVATE ./src/core)

if(WIN32)
    file(GLOB_RECURSE ALL_RC "rc/gen/rc/*.rc")
    if(EXISTS ALL_RC)
        target_sources(${PROJECT_NAME} PRIVATE ${ALL_RC})
    endif()
endif(WIN32)

# 控制台仅在 Debug 模式下生成
if (${CMAKE_BUILD_TYPE} MATCHES ".*(D|d)(E|e)(B|b)(U|u)(G|g).*")
    message(STATUS "${PROJECT_NAME}::CMakeLists.txt::info: 将编译带有控制台的目标程序")
else()
    message(STATUS "${PROJECT_NAME}::CMakeLists.txt::info: 将编译隐藏控制台的目标程序")
    target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
endif()

# 链接 imgui
target_include_directories(${PROJECT_NAME} PUBLIC ../3rd/imgui/source/include)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)

# 链接 util
target_include_directories(${PROJECT_NAME} PUBLIC ../util/include)
target_link_libraries(${PROJECT_NAME} PUBLIC util)

# 链接 rc
target_include_directories(${PROJECT_NAME} PUBLIC rc/gen/include)
target_link_libraries(${PROJECT_NAME} PRIVATE rc)