include(FetchContent)

if (NOT GRPCXX_USE_ASIO)
	# libuv
	FetchContent_Declare(libuv
		URL      https://github.com/libuv/libuv/archive/refs/tags/v1.46.0.tar.gz
		URL_HASH SHA256=7aa66be3413ae10605e1f5c9ae934504ffe317ef68ea16fdaa83e23905c681bd
	)

	set(LIBUV_BUILD_SHARED OFF CACHE BOOL "Build libuv shared lib")
	FetchContent_MakeAvailable(libuv)
	add_library(libuv::uv ALIAS uv_a)
else ()
	# asio
	find_path(Asio_INCLUDE_DIR NAMES asio.hpp)
	if (Asio_INCLUDE_DIR)
		file(READ "${Asio_INCLUDE_DIR}/asio/version.hpp" tmp_version)
		string(REGEX MATCH "#define ASIO_VERSION ([0-9]+)" REGEX_VERSION ${tmp_version})

		set(tmp_asio_version ${CMAKE_MATCH_1})
		math(EXPR Asio_VERSION_MAJOR "${tmp_asio_version} / 100000")
		math(EXPR Asio_VERSION_MINOR "${tmp_asio_version} / 100 % 1000")
		math(EXPR Asio_VERSION_PATCH "${tmp_asio_version} % 100")
		set(Asio_VERSION "${Asio_VERSION_MAJOR}.${Asio_VERSION_MINOR}.${Asio_VERSION_PATCH}")

		unset(tmp_version)
		unset(tmp_asio_version)

		if (${Asio_VERSION} VERSION_LESS 1.28)
			unset(Asio_INCLUDE_DIR)
			unset(Asio_VERSION)
			unset(Asio_VERSION_MAJOR)
			unset(Asio_VERSION_MINOR)
			unset(Asio_VERSION_PATCH)
		else()
			set(Asio_FOUND ON)
			message(STATUS "Found Asio ${Asio_VERSION} at ${Asio_INCLUDE_DIR}")
		endif()
	endif()

	if (NOT Asio_FOUND)
		FetchContent_Declare(asio
			URL      https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-29-0.tar.gz
			URL_HASH SHA256=44305859b4e6664dbbf853c1ef8ca0259d694f033753ae309fcb2534ca20f721
		)
		FetchContent_MakeAvailable(asio)

		set(Asio_INCLUDE_DIR "$<BUILD_INTERFACE:${asio_SOURCE_DIR}/asio/include>")
	endif()

	add_library(asio INTERFACE)
	target_include_directories(asio INTERFACE ${Asio_INCLUDE_DIR})
	target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
endif()

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

# protobuf
find_package(Protobuf 3.15.0 REQUIRED)

# fmt
FetchContent_Declare(fmt
	URL      https://github.com/fmtlib/fmt/archive/refs/tags/10.1.1.tar.gz
	URL_HASH SHA256=78b8c0a72b1c35e4443a7e308df52498252d1cefc2b08c9a97bc9ee6cfe61f8b
)
FetchContent_MakeAvailable(fmt)
