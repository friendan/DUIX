# lunasvg + plutovg（供 DuiLib SvgBox 等使用）
# 不直接 add_subdirectory 上游 CMakeLists，避免二次 project()/install 干扰主工程

set(LUNASVG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lunasvg-3.5.0")
set(PLUTOVG_ROOT "${LUNASVG_ROOT}/plutovg")

# --- plutovg (C) ---
set(plutovg_sources
	${PLUTOVG_ROOT}/source/plutovg-blend.c
	${PLUTOVG_ROOT}/source/plutovg-canvas.c
	${PLUTOVG_ROOT}/source/plutovg-font.c
	${PLUTOVG_ROOT}/source/plutovg-matrix.c
	${PLUTOVG_ROOT}/source/plutovg-paint.c
	${PLUTOVG_ROOT}/source/plutovg-path.c
	${PLUTOVG_ROOT}/source/plutovg-rasterize.c
	${PLUTOVG_ROOT}/source/plutovg-surface.c
	${PLUTOVG_ROOT}/source/plutovg-ft-math.c
	${PLUTOVG_ROOT}/source/plutovg-ft-raster.c
	${PLUTOVG_ROOT}/source/plutovg-ft-stroker.c
)

add_library(plutovg STATIC ${plutovg_sources})
add_library(plutovg::plutovg ALIAS plutovg)

set_target_properties(plutovg PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED ON
	C_VISIBILITY_PRESET hidden
)

target_include_directories(plutovg
	PUBLIC  ${PLUTOVG_ROOT}/include
	PRIVATE ${PLUTOVG_ROOT}/source
)

target_compile_definitions(plutovg
	PRIVATE PLUTOVG_BUILD
	PUBLIC  PLUTOVG_BUILD_STATIC
)

# --- lunasvg (C++17) ---
set(lunasvg_sources
	${LUNASVG_ROOT}/source/lunasvg.cpp
	${LUNASVG_ROOT}/source/graphics.cpp
	${LUNASVG_ROOT}/source/svgelement.cpp
	${LUNASVG_ROOT}/source/svggeometryelement.cpp
	${LUNASVG_ROOT}/source/svglayoutstate.cpp
	${LUNASVG_ROOT}/source/svgpaintelement.cpp
	${LUNASVG_ROOT}/source/svgparser.cpp
	${LUNASVG_ROOT}/source/svgproperty.cpp
	${LUNASVG_ROOT}/source/svgrenderstate.cpp
	${LUNASVG_ROOT}/source/svgtextelement.cpp
)

add_library(lunasvg STATIC ${lunasvg_sources})
add_library(lunasvg::lunasvg ALIAS lunasvg)

set_target_properties(lunasvg PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_VISIBILITY_PRESET hidden
)

target_include_directories(lunasvg
	PUBLIC  ${LUNASVG_ROOT}/include
	PRIVATE ${LUNASVG_ROOT}/source
)

target_compile_definitions(lunasvg
	PRIVATE LUNASVG_BUILD
	PUBLIC  LUNASVG_BUILD_STATIC
)

target_link_libraries(lunasvg PUBLIC plutovg::plutovg)

if(MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	target_compile_options(lunasvg PRIVATE /W3 /WX- /utf-8)
	target_compile_options(plutovg PRIVATE /W3 /WX- /utf-8)
endif()
