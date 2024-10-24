include(FetchContent)

set(LIBUV_MINVERSION 1.46.0)
set(BOOST_MINVERSION 1.81)
set(NGHTTP2_MINVERSION 1.55.1)
set(PROTOBUF_MINVERSION 3.15.0)
set(FMT_MINVERSION 10.1.1)

if(NOT GRPCXX_USE_ASIO)
	if(GRPCXX_HERMETIC_BUILD)
		# libuv
		FetchContent_Declare(libuv
			URL      https://github.com/libuv/libuv/archive/refs/tags/v${LIBUV_MINVERSION}.tar.gz
			URL_HASH SHA256=7aa66be3413ae10605e1f5c9ae934504ffe317ef68ea16fdaa83e23905c681bd
		)

		set(LIBUV_BUILD_SHARED OFF CACHE BOOL "Build libuv shared lib")
		FetchContent_MakeAvailable(libuv)
		install(TARGETS uv_a EXPORT grpcxx COMPONENT Development)
		add_library(libuv::uv ALIAS uv_a)
	else()
		# Unfortunately the libuv CMakeLists.txt does not export
		# a version file. This is a bug in libuv.
		# So we cannot use ${LIBUV_MINVERSION} in find_package().
		# To work around it, we do a pkg_config call just to check
		# the version.
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(uv REQUIRED "libuv>=${LIBUV_MINVERSION}")
		find_package(libuv REQUIRED)
	endif()
else()
	# asio
	add_library(asio INTERFACE)
	install(TARGETS asio EXPORT grpcxx COMPONENT Development)

	find_package(Boost ${BOOST_MINVERSION})
	if (Boost_FOUND)
		message(STATUS "Found Boost ${Boost_VERSION} at ${Boost_INCLUDE_DIR}")

		target_include_directories(asio INTERFACE ${Boost_INCLUDE_DIR})
		target_compile_definitions(asio
			INTERFACE
				ASIO_NS=::boost::asio
				BOOST_ASIO_STANDALONE
				BOOST_ASIO_NO_DEPRECATED
		)
	else()
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

		if (NOT Asio_FOUND AND GRPCXX_HERMETIC_BUILD)
			FetchContent_Declare(asio
				URL      https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-29-0.tar.gz
				URL_HASH SHA256=44305859b4e6664dbbf853c1ef8ca0259d694f033753ae309fcb2534ca20f721
			)
			FetchContent_MakeAvailable(asio)

			set(Asio_INCLUDE_DIR "$<BUILD_INTERFACE:${asio_SOURCE_DIR}/asio/include>")
		endif()

		target_include_directories(asio INTERFACE ${Asio_INCLUDE_DIR})
		target_compile_definitions(asio
			INTERFACE
				ASIO_NS=::asio
				ASIO_STANDALONE
				ASIO_NO_DEPRECATED
		)
	endif()
endif()

# nghttp2
if(NOT GRPCXX_HERMETIC_BUILD)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(nghttp2 REQUIRED IMPORTED_TARGET "libnghttp2>=${NGHTTP2_MINVERSION}")
	add_library(libnghttp2::nghttp2 ALIAS PkgConfig::nghttp2)
else()
	FetchContent_Declare(nghttp2
		URL      https://github.com/nghttp2/nghttp2/releases/download/v${NGHTTP2_MINVERSION}/nghttp2-${NGHTTP2_MINVERSION}.tar.xz
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

	install(TARGETS nghttp2_static EXPORT grpcxx COMPONENT Development)
	add_library(libnghttp2::nghttp2 ALIAS nghttp2_static)
endif()

# protobuf
find_package(Protobuf ${PROTOBUF_MINVERSION} REQUIRED)

if(NOT GRPCXX_HERMETIC_BUILD)
	find_package(fmt ${FMT_MINVERSION} REQUIRED)
else()
	# fmt
	FetchContent_Declare(fmt
		URL      https://github.com/fmtlib/fmt/archive/refs/tags/${FMT_MINVERSION}.tar.gz
		URL_HASH SHA256=78b8c0a72b1c35e4443a7e308df52498252d1cefc2b08c9a97bc9ee6cfe61f8b
	)
	FetchContent_MakeAvailable(fmt)
endif()
