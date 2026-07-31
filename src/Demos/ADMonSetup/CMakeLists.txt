project(ADMonSetup)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/ADMonSetup.rc)

add_executable(ADMonSetup WIN32 ${SOURCE_FILES})
target_link_libraries(ADMonSetup DuiLib)