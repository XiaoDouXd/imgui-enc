cmake_minimum_required(VERSION 3.25)
project(rc)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if(WIN32)
    file (GLOB_RECURSE ALL_RC "./rc/*.in")
    foreach(rc_in ${ALL_RC})
        file(RELATIVE_PATH rc_in_name "${CMAKE_CURRENT_SOURCE_DIR}/rc" ${rc_in})
        string(REGEX REPLACE ".in$" "" rc_out ${rc_in_name})
        configure_file(${rc_in} "${CMAKE_CURRENT_SOURCE_DIR}/gen/rc/${rc_out}")
    endforeach()
endif(WIN32)

if(CC_RC_REGEN)
    # 需要生成嵌入的文件
    file (GLOB_RECURSE ALL_RES "./res/*")

    # 输出文件目录
    set (GEN_RES_OUTPUT_HDR_DIR "./gen/include")
    set (GEN_RES_OUTPUT_SRC_DIR "./gen/src")
    file(REMOVE_RECURSE ${GEN_RES_OUTPUT_HDR_DIR})
    file(REMOVE_RECURSE ${GEN_RES_OUTPUT_SRC_DIR})
    file(MAKE_DIRECTORY ${GEN_RES_OUTPUT_HDR_DIR})
    file(MAKE_DIRECTORY ${GEN_RES_OUTPUT_SRC_DIR})

    # 依次处理文件
    foreach(input_src ${ALL_RES})
        set(suffix "")

        # 配置输出文件名
        file(SIZE ${input_src} res_file_size)
        file(RELATIVE_PATH res_file "${CMAKE_CURRENT_SOURCE_DIR}/res" ${input_src})

        string(REGEX MATCH ".*\\[(\\(([0-9]+)\\))?(.*)\\](\\.(.*))?$" _ ${res_file})
        set(suffix ${CMAKE_MATCH_3})
        if(NOT suffix STREQUAL "" AND NOT CMAKE_MATCH_2 STREQUAL "")
            math(EXPR res_file_size "${CMAKE_MATCH_2} + ${res_file_size}" OUTPUT_FORMAT DECIMAL)
        endif()

        # 把所有非标识符字符替换为 "_"
        string(MAKE_C_IDENTIFIER ${res_file} token)
        string(TOUPPER ${token} token)
        set(gen_res_file        "${GEN_RES_OUTPUT_SRC_DIR}/${token}.cpp")
        set(gen_res_file_header "${GEN_RES_OUTPUT_HDR_DIR}/${token}.h")

        file(APPEND ${gen_res_file_header}
        "#pragma once\n\n"
        "#include <array>\n#include <cstdint>\n\n"
        "namespace CC::RC {\n"
        "    extern const std::array<uint8_t, ${res_file_size}> CC_RC_${token};\n"
        "}")
        file(APPEND ${gen_res_file}
            "#include \"${token}.h\"\n\n"
            "namespace CC::RC {\n"
            "    const std::array<uint8_t, ${res_file_size}> CC_RC_${token} = {\n       ")

        # 读取十六进制数据
        file(READ ${input_src} file_data HEX)
        if(CC_RC_CROSSLINE)
            # 把十六进制数据表示方式换成 C/C++ 风格
            string(REGEX REPLACE "([0-9a-f][0-9a-f])" " 0x\\1,;" file_data ${file_data})
            set(i 0)
            foreach(single_data ${file_data})
                file(APPEND ${gen_res_file} ${single_data})
                if(${i} EQUAL 20)
                    file(APPEND ${gen_res_file} "\n       ")
                    set(i 0)
                endif()
                math(EXPR i "1 + ${i}" OUTPUT_FORMAT DECIMAL)
            endforeach()
        else()
            # 把十六进制数据表示方式换成 C/C++ 风格
            string(REGEX REPLACE "([0-9a-f][0-9a-f])" " 0x\\1," file_data ${file_data})
            file(APPEND ${gen_res_file} ${file_data})
        endif(CC_RC_CROSSLINE)

        # 写入数据到源/头文件
        file(APPEND ${gen_res_file}
            " ${suffix}\n    };\n}"
        )
    endforeach()
endif(CC_RC_REGEN)

file(GLOB_RECURSE GEN_RES_FILES "gen/*.cpp" "gen/*.c" "gen/*.h" "gen/*.hpp")
include_directories(${PROJECT_NAME} ./gen ./gen/include)
add_library(${PROJECT_NAME} STATIC ${GEN_RES_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${GEN_RES_OUTPUT_HDR_DIR})