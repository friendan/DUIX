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

# --- zlib (OBJECT lib, 合并进 DuiLib) ---
set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.3.2")
add_library(zlib OBJECT
	${ZLIB_ROOT}/adler32.c
	${ZLIB_ROOT}/compress.c
	${ZLIB_ROOT}/crc32.c
	${ZLIB_ROOT}/deflate.c
	${ZLIB_ROOT}/gzclose.c
	${ZLIB_ROOT}/gzlib.c
	${ZLIB_ROOT}/gzread.c
	${ZLIB_ROOT}/gzwrite.c
	${ZLIB_ROOT}/inflate.c
	${ZLIB_ROOT}/infback.c
	${ZLIB_ROOT}/inftrees.c
	${ZLIB_ROOT}/inffast.c
	${ZLIB_ROOT}/trees.c
	${ZLIB_ROOT}/uncompr.c
	${ZLIB_ROOT}/zutil.c
)
target_include_directories(zlib PRIVATE ${ZLIB_ROOT})
if(MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	target_compile_options(zlib PRIVATE /W3 /WX-)
endif()

# --- minizip-ng (OBJECT lib, 合并进 DuiLib) ---
# 复用仓库内已按 Windows 静态编译维护好的 mz_config.h，不跑上游 configure_file。
set(MINIZIP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/minizip-ng-4.2.2")
add_library(minizip_ng OBJECT
	${MINIZIP_ROOT}/mz_crypt.c
	${MINIZIP_ROOT}/mz_crypt_winvista.c
	${MINIZIP_ROOT}/mz_crypt_winxp.c
	${MINIZIP_ROOT}/mz_os.c
	${MINIZIP_ROOT}/mz_os_win32.c
	${MINIZIP_ROOT}/mz_strm.c
	${MINIZIP_ROOT}/mz_strm_buf.c
	${MINIZIP_ROOT}/mz_strm_mem.c
	${MINIZIP_ROOT}/mz_strm_os_win32.c
	${MINIZIP_ROOT}/mz_strm_pkcrypt.c
	${MINIZIP_ROOT}/mz_strm_split.c
	${MINIZIP_ROOT}/mz_strm_wzaes.c
	${MINIZIP_ROOT}/mz_strm_zlib.c
	${MINIZIP_ROOT}/mz_zip.c
	${MINIZIP_ROOT}/mz_zip_rw.c
)
target_include_directories(minizip_ng PRIVATE ${MINIZIP_ROOT} ${ZLIB_ROOT})
target_compile_definitions(minizip_ng PRIVATE
	HAVE_ZLIB HAVE_WZAES HAVE_BCRYPT HAVE_PKCRYPT ZLIB_COMPAT
	_CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE
)
if(MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	target_compile_options(minizip_ng PRIVATE /W3 /WX-)
endif()
