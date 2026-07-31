project(WkeBrowser)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SOURCE_FILES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/WkeBrowser.rc)

add_executable(WkeBrowser WIN32 ${SOURCE_FILES})
target_include_directories(WkeBrowser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/Wke/Include)
target_link_libraries(WkeBrowser DuiLib)

find_library(WKE_LIB NAMES wke PATHS ${CMAKE_CURRENT_SOURCE_DIR}/3rd/Wke/Lib)
if(WKE_LIB)
    target_link_libraries(WkeBrowser ${WKE_LIB})
endif()

find_library(TROY_CONTROLS_LIB
    NAMES TroyControls TroyControls_d
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/Lib)
if(TROY_CONTROLS_LIB)
    target_link_libraries(WkeBrowser ${TROY_CONTROLS_LIB})
endif()