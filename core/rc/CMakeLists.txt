cmake_minimum_required(VERSION 3.25)
project(rc)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

file(GLOB_RECURSE ALL_RES "./res/*")
if(DEFINED XD_RC_PATH AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../${XD_RC_PATH}")
    set(__RC_PATH_REAL "../../${XD_RC_PATH}")
    get_filename_component(__RC_PATH_REAL_FULLPATH "${__RC_PATH_REAL}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
    if(WIN32 AND DEFINED XD_RC_WINDRES_PATH AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../${XD_RC_WINDRES_PATH}")
        set(XD_RC_WINDRES_PATH "../../${XD_RC_WINDRES_PATH}")
        get_filename_component(__RC_WINDRES_PATH_REAL_FULLPATH "${XD_RC_WINDRES_PATH}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
        file (GLOB_RECURSE ALL_RC "../${XD_RC_WINDRES_PATH}/*.rc.in")
        foreach(rc_in ${ALL_RC})
            file(RELATIVE_PATH rc_in_name "${__RC_WINDRES_PATH_REAL_FULLPATH}" ${rc_in})
            string(REGEX REPLACE ".in$" "" rc_out ${rc_in_name})
            configure_file(${rc_in} "${CMAKE_CURRENT_SOURCE_DIR}/gen/rc/${rc_out}")
        endforeach()
    endif()
    file(GLOB_RECURSE __ALL_RES_OUTSIDE "../${__RC_PATH_REAL}/*")
endif()

if(XD_RC_REGEN)
    # 输出文件目录
    set (GEN_RES_OUTPUT_HDR_DIR "./gen/include")
    set (GEN_RES_OUTPUT_SRC_DIR "./gen/src")
    file(REMOVE_RECURSE ${GEN_RES_OUTPUT_HDR_DIR})
    file(REMOVE_RECURSE ${GEN_RES_OUTPUT_SRC_DIR})
    file(MAKE_DIRECTORY ${GEN_RES_OUTPUT_HDR_DIR})
    file(MAKE_DIRECTORY ${GEN_RES_OUTPUT_SRC_DIR})

    set (GEN_RES_OUTPUT_HDR_DIR_XD "./gen/include_in/xdres")
    set (GEN_RES_OUTPUT_SRC_DIR_XD "./gen/src_in")
    file(REMOVE_RECURSE ${GEN_RES_OUTPUT_HDR_DIR_XD})
    file(REMOVE_RECURSE ${GEN_RES_OUTPUT_SRC_DIR_XD})
    file(MAKE_DIRECTORY ${GEN_RES_OUTPUT_HDR_DIR_XD})
    file(MAKE_DIRECTORY ${GEN_RES_OUTPUT_SRC_DIR_XD})

    # ========================================================================= 依次处理文件
    # ------------------------------------------------------------------------- 处理内部的文件
    foreach(input_src ${ALL_RES})
        set(suffix "")

        # 配置输出文件名
        file(SIZE ${input_src} res_file_size)
        file(RELATIVE_PATH res_file "${CMAKE_CURRENT_SOURCE_DIR}/" ${input_src})

        string(REGEX MATCH ".*\\[(\\(([0-9]+)\\))?(.*)\\](\\.(.*))?$" _ ${res_file})
        set(suffix ${CMAKE_MATCH_3})
        if(NOT suffix STREQUAL "" AND NOT CMAKE_MATCH_2 STREQUAL "")
            math(EXPR res_file_size "${CMAKE_MATCH_2} + ${res_file_size}" OUTPUT_FORMAT DECIMAL)
        endif()

        # 把所有非标识符字符替换为 "_"
        string(MAKE_C_IDENTIFIER ${res_file} token)
        string(TOUPPER ${token} token)
        set(gen_res_file        "${GEN_RES_OUTPUT_SRC_DIR_XD}/${token}.cpp")
        set(gen_res_file_header "${GEN_RES_OUTPUT_HDR_DIR_XD}/${token}.h")

        file(APPEND ${gen_res_file_header}
        "#pragma once\n\n"
        "#include <array>\n#include <cstdint>\n\n"
        "namespace XD::RC {\n"
        "    extern const std::array<uint8_t, ${res_file_size}> XD_IN_RC_${token};\n"
        "}")
        file(APPEND ${gen_res_file}
            "#include \"xdres/${token}.h\"\n\n"
            "namespace XD::RC {\n"
            "    const std::array<uint8_t, ${res_file_size}> XD_IN_RC_${token} = {\n       ")

        # 读取十六进制数据
        file(READ ${input_src} file_data HEX)
        if(XD_RC_CROSSLINE)
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
        endif(XD_RC_CROSSLINE)

        # 写入数据到源/头文件
        file(APPEND ${gen_res_file}
            " ${suffix}\n    };\n}"
        )
    endforeach()

    # ------------------------------------------------------------------------- 处理外部的文件
    if (DEFINED __ALL_RES_OUTSIDE)
        foreach(input_src ${__ALL_RES_OUTSIDE})
            set(suffix "")

            # 配置输出文件名
            file(SIZE ${input_src} res_file_size)
            file(RELATIVE_PATH res_file "${__RC_PATH_REAL_FULLPATH}/" ${input_src})

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
            "namespace XD::RC {\n"
            "    extern const std::array<uint8_t, ${res_file_size}> XD_RC_${token};\n"
            "}")
            file(APPEND ${gen_res_file}
                "#include \"${token}.h\"\n\n"
                "namespace XD::RC {\n"
                "    const std::array<uint8_t, ${res_file_size}> XD_RC_${token} = {\n       ")

            # 读取十六进制数据
            file(READ ${input_src} file_data HEX)
            if(XD_RC_CROSSLINE)
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
            endif(XD_RC_CROSSLINE)

            # 写入数据到源/头文件
            file(APPEND ${gen_res_file}
                " ${suffix}\n    };\n}"
            )
        endforeach()
    endif()
endif(XD_RC_REGEN)

file(GLOB_RECURSE GEN_RES_FILES "gen/*.cpp" "gen/*.c" "gen/*.h" "gen/*.hpp")
include_directories(${PROJECT_NAME} ./gen ./gen/include)
add_library(${PROJECT_NAME} STATIC ${GEN_RES_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${GEN_RES_OUTPUT_HDR_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ./gen/include_in)


