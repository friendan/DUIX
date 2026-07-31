project(bdwallpaper)

file(GLOB ROOT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB HELPER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/helper/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/helper/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/helper/jsoncpp/*.cpp)
list(FILTER HELPER_SOURCES EXCLUDE REGEX "stb_image\\.c$")
list(FILTER HELPER_SOURCES EXCLUDE REGEX "XUnzip\\.cpp$")
set(SOURCE_FILES ${ROOT_SOURCES} ${HELPER_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/BDWallPaper.rc)

add_executable(bdwallpaper WIN32 ${SOURCE_FILES})
target_include_directories(bdwallpaper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/controlex
    ${CMAKE_CURRENT_SOURCE_DIR}/helper
    ${CMAKE_CURRENT_SOURCE_DIR}/helper/jsoncpp)
target_link_libraries(bdwallpaper DuiLib)