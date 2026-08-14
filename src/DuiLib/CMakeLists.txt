aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} RootSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Control ControlSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Core CoreSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Layout LayoutSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Utils UtilsSrc)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Icons IconsSrc)

add_library(DuiLib STATIC
    ${RootSrc} ${ControlSrc} ${CoreSrc} ${LayoutSrc} ${UtilsSrc} ${IconsSrc}
    $<TARGET_OBJECTS:zlib>
    $<TARGET_OBJECTS:minizip_ng>
)

target_include_directories(DuiLib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Control
        ${CMAKE_CURRENT_SOURCE_DIR}/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/Layout
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils
        ${CMAKE_CURRENT_SOURCE_DIR}/Icons
        ${CMAKE_SOURCE_DIR}/3rd/minizip-ng-4.2.2
        ${CMAKE_SOURCE_DIR}/3rd/zlib-1.3.2
)

target_compile_definitions(DuiLib
    PUBLIC UILIB_STATIC
    PRIVATE HAVE_ZLIB HAVE_WZAES HAVE_BCRYPT ZLIB_COMPAT
)

# 编译期默认后端：ON=Direct2D，OFF=GDI（仍可运行时 EnableGdi/EnableD2d 切换）
option(DUILIB_USE_D2D "Compile with Direct2D as default render backend" ON)
if(DUILIB_USE_D2D)
    target_compile_definitions(DuiLib PUBLIC DUILIB_RENDER_BACKEND=1)
endif()

# WebView2（Edge 嵌入浏览器）。SDK 默认：src/3rd/webview2/pkg/build/native
option(DUILIB_USE_WEBVIEW2 "Enable WebView2 browser engine" ON)
set(WEBVIEW2_ROOT "${CMAKE_SOURCE_DIR}/3rd/webview2/pkg/build/native" CACHE PATH "WebView2 native SDK root")
if(DEFINED ENV{WEBVIEW2_SDK_PATH} AND NOT "$ENV{WEBVIEW2_SDK_PATH}" STREQUAL "")
    set(WEBVIEW2_ROOT "$ENV{WEBVIEW2_SDK_PATH}" CACHE PATH "WebView2 native SDK root" FORCE)
endif()
if(DUILIB_USE_WEBVIEW2)
    if(EXISTS "${WEBVIEW2_ROOT}/include/WebView2.h")
        target_compile_definitions(DuiLib PUBLIC DUILIB_HAS_WEBVIEW2=1)
        target_include_directories(DuiLib PUBLIC "${WEBVIEW2_ROOT}/include")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_WV2_ARCH x64)
        else()
            set(_WV2_ARCH x86)
        endif()
        target_link_libraries(DuiLib PUBLIC
            "${WEBVIEW2_ROOT}/${_WV2_ARCH}/WebView2LoaderStatic.lib"
            version
        )
        message(STATUS "WebView2 enabled (static loader): ${WEBVIEW2_ROOT}")
    else()
        message(WARNING "DUILIB_USE_WEBVIEW2=ON but WebView2.h not found under ${WEBVIEW2_ROOT}; building without WebView2")
        set(DUILIB_USE_WEBVIEW2 OFF CACHE BOOL "Enable WebView2 browser engine" FORCE)
    endif()
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
