project(TroyBrowser)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/TroyBrowser.rc)

add_executable(TroyBrowser WIN32 ${SOURCE_FILES})
target_include_directories(TroyBrowser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TroyBrowser DuiLib)

find_library(TROY_CONTROLS_LIB
    NAMES TroyControls TroyControls_d
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/Lib)
if(TROY_CONTROLS_LIB)
    target_link_libraries(TroyBrowser ${TROY_CONTROLS_LIB})
endif()