project(HiDPITest)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/HiDPITest.rc)

add_executable(HiDPITest WIN32 ${SOURCE_FILES})
target_link_libraries(HiDPITest DuiLib)