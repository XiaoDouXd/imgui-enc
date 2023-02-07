cmake_minimum_required(VERSION 3.25)
project(imgui)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

file(GLOB_RECURSE IMGUI_MISC_SOURCE "source/misc/*.cpp" "source/misc/*.h" "source/misc/*.hpp" "source/misc/*.c")
file(GLOB_RECURSE IMGUI_MISC_IMPL_FIX "implFix/*.cpp" "implFix/*.c" "implFix/*.hpp" "implFix/*.h")
set(IMGUI_BACKENDS_SOURCE
    "source/backends/imgui_impl_vulkan.h"
    "source/backends/imgui_impl_vulkan.cpp"
)
set(IMGUI_SOURCE
    "source/imgui.h"
    "source/imgui.cpp"
    "source/imgui_widgets.cpp"
    "source/imgui_tables.cpp"
    "source/imgui_internal.h"
    "source/imgui_draw.cpp"
    "source/imstb_rectpack.h"
    "source/imstb_textedit.h"
    "source/imstb_truetype.h"
    "source/imconfig.h"

    # demo 窗口试运行
    "source/imgui_demo.cpp"
)

if(${ANALYZER_DYNAMIC})
    message(STATUS ${PROJECT_NAME} "::CMakeLists.txt::info: 分析器将编译为动态库.")
    add_library(${PROJECT_NAME} SHARED ${IMGUI_SOURCE} ${IMGUI_BACKENDS_SOURCE} ${IMGUI_MISC_SOURCE} ${IMGUI_MISC_IMPL_FIX})
else()
    message(STATUS ${PROJECT_NAME} "::CMakeLists.txt::info: 分析器将编译为静态库.")
    add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCE} ${IMGUI_BACKENDS_SOURCE} ${IMGUI_MISC_SOURCE} ${IMGUI_MISC_IMPL_FIX})
endif(${ANALYZER_DYNAMIC})
target_include_directories(${PROJECT_NAME} PUBLIC "./implFix")
target_include_directories(${PROJECT_NAME} PUBLIC "./source")
target_include_directories(${PROJECT_NAME} PUBLIC "./source/backends")

# -------------------------------------------------- 外部库
# 添加 vulkan
find_package(Vulkan REQUIRED)
message(STATUS ${PROJECT_NAME} "::CMakeLists::info: include vulkan from \"" ${Vulkan_INCLUDE_DIRS} "\"")
message(STATUS ${PROJECT_NAME} "::CMakeLists::info: link vulkan lib from \"" ${Vulkan_LIBRARIES} "\"")
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})

# 添加 SDL
target_include_directories(${PROJECT_NAME} PUBLIC ../SDL3/include/SDL3)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3-static)

# 添加 freetype
find_package(Freetype REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${FREETYPE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})