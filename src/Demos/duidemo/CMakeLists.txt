project(duidemo)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/duidemo.rc)

add_executable(duidemo WIN32 ${SOURCE_FILES})
target_link_libraries(duidemo DuiLib)