include(FetchContent)

# libuv
FetchContent_Declare(libuv
	URL      https://github.com/libuv/libuv/archive/refs/tags/v1.46.0.tar.gz
	URL_HASH SHA256=7aa66be3413ae10605e1f5c9ae934504ffe317ef68ea16fdaa83e23905c681bd
)

set(LIBUV_BUILD_SHARED OFF CACHE BOOL "Build libuv shared lib")

FetchContent_MakeAvailable(libuv)

add_library(libuv::uv ALIAS uv_a)

# nghttp2
FetchContent_Declare(nghttp2
	URL      https://github.com/nghttp2/nghttp2/releases/download/v1.55.1/nghttp2-1.55.1.tar.xz
	URL_HASH SHA256=19490b7c8c2ded1cf7c3e3a54ef4304e3a7876ae2d950d60a81d0dc6053be419
)

set(ENABLE_LIB_ONLY   ON  CACHE BOOL "Build libnghttp2 only")
set(ENABLE_STATIC_LIB ON  CACHE BOOL "Build libnghttp2 in static mode")
set(ENABLE_SHARED_LIB OFF CACHE BOOL "Build libnghttp2 as a shared library")
set(ENABLE_DOC        OFF CACHE BOOL "Build libnghttp2 documentation")

FetchContent_MakeAvailable(nghttp2)

target_include_directories(nghttp2_static
	PUBLIC
		$<BUILD_INTERFACE:${nghttp2_SOURCE_DIR}/lib/includes>
)
add_library(libnghttp2::nghttp2 ALIAS nghttp2_static)
