cmake_minimum_required(VERSION 3.10)

# 静态 CRT（须在 project() 前设好策略）
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(DuiLib VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DCMAKE -DUNICODE -D_UNICODE)

if(MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(/W3 /WX- /utf-8)
endif()

# 兼容旧缓存 / 未走 MSVC_RUNTIME_LIBRARY 的生成器：Debug=/MTd，Release=/MT
foreach(_flag_var
    CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(DEFINED ${_flag_var})
        string(REGEX REPLACE "/MD" "/MT" ${_flag_var} "${${_flag_var}}")
    endif()
endforeach()

# 产物后缀：Debug → *_mtd，Release → *_mt
set(CMAKE_DEBUG_POSTFIX "_mtd")
set(CMAKE_RELEASE_POSTFIX "_mt")
set(CMAKE_MINSIZEREL_POSTFIX "_mt")
set(CMAKE_RELWITHDEBINFO_POSTFIX "_mt")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)

add_subdirectory(3rd)
add_subdirectory(DuiLib)
add_subdirectory(Demos)

# CMAKE_*_POSTFIX 默认只作用于库；可执行文件需显式设置
function(duilib_apply_crt_postfix_to_exes dir)
    get_property(_targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_tgt IN LISTS _targets)
        get_target_property(_type ${_tgt} TYPE)
        if(_type STREQUAL "EXECUTABLE")
            set_target_properties(${_tgt} PROPERTIES
                DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
                RELEASE_POSTFIX "${CMAKE_RELEASE_POSTFIX}"
                MINSIZEREL_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}"
                RELWITHDEBINFO_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}"
            )
        endif()
    endforeach()
    get_property(_subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        duilib_apply_crt_postfix_to_exes("${_subdir}")
    endforeach()
endfunction()
duilib_apply_crt_postfix_to_exes("${CMAKE_CURRENT_SOURCE_DIR}/Demos")
