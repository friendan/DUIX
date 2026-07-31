project(gamebox)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
list(FILTER SOURCES EXCLUDE REGEX "stb_image\\.c$")
list(FILTER SOURCES EXCLUDE REGEX "XUnzip\\.cpp$")
set(SOURCE_FILES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/GameBox.rc)

add_executable(gamebox WIN32 ${SOURCE_FILES})
target_link_libraries(gamebox DuiLib)