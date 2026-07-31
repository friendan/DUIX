aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} RootSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Control ControlSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core CoreSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Layout LayoutSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Utils UtilsSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Icons IconsSrc)

add_library(DuiLib STATIC ${RootSrc} ${ControlSrc} ${CoreSrc} ${LayoutSrc} ${UtilsSrc} ${IconsSrc})

target_include_directories(DuiLib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Control
        ${CMAKE_CURRENT_SOURCE_DIR}/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/Layout
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons
)

target_compile_definitions(DuiLib PUBLIC UILIB_STATIC)

# 编译期默认后端：ON=Direct2D，OFF=GDI（仍可运行时 EnableGdi/EnableD2d 切换）
option(DUILIB_USE_D2D "Compile with Direct2D as default render backend" ON)
if(DUILIB_USE_D2D)
    target_compile_definitions(DuiLib PUBLIC DUILIB_RENDER_BACKEND=1)
endif()

target_link_libraries(DuiLib PUBLIC d2d1 dwrite d3d11 dxgi dcomp lunasvg::lunasvg)

# 巨型嵌入图标表需要 /bigobj
if(MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons/BootstrapIconsData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons/LucideIconsIconsData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons/IconParkIconsData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons/TablerOutlineIconsData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons/TablerFilledIconsData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons/RemixIconIconsData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons/TwemojiIconsData.cpp
        PROPERTIES COMPILE_FLAGS "/bigobj"
    )
endif()
