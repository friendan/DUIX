project(transwnd)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/transwnd.rc)

add_executable(transwnd WIN32 ${SOURCE_FILES})
target_link_libraries(transwnd DuiLib)