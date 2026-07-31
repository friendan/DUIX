cmake_minimum_required(VERSION 3.10)

project(DuiLib VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DCMAKE -DUNICODE -D_UNICODE)

if(MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(/W3 /WX- /utf-8)
endif()

set(CMAKE_DEBUG_POSTFIX "_dbg")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)

add_subdirectory(3rd)
add_subdirectory(DuiLib)
add_subdirectory(Demos)

# CMAKE_DEBUG_POSTFIX 默认只作用于库；可执行文件需显式设置（Debug → xxx_dbg.exe）
function(duilib_apply_debug_postfix_to_exes dir)
    get_property(_targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_tgt IN LISTS _targets)
        get_target_property(_type ${_tgt} TYPE)
        if(_type STREQUAL "EXECUTABLE")
            set_target_properties(${_tgt} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
        endif()
    endforeach()
    get_property(_subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        duilib_apply_debug_postfix_to_exes("${_subdir}")
    endforeach()
endfunction()
duilib_apply_debug_postfix_to_exes("${CMAKE_CURRENT_SOURCE_DIR}/Demos")
