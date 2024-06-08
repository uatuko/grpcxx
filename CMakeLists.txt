cmake_minimum_required(VERSION 3.23)
project(grpcxx VERSION 0.2.0 LANGUAGES CXX)

cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0144 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

option(GRPCXX_USE_ASIO "Use asio instead of libuv for I/O and event loop" OFF)

include(CMakeDependentOption)
cmake_dependent_option(GRPCXX_BUILD_EXAMPLES
	"Build examples when this is the root project" ON
	"CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
)

cmake_dependent_option(GRPCXX_BUILD_EXPERIMENTS
	"Build experiments when this is the root project" ON
	"NOT GRPCXX_USE_ASIO; CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
)

include(cmake/dependencies.cmake)

if (GRPCXX_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if (GRPCXX_BUILD_EXPERIMENTS)
	add_subdirectory(.experiments)
endif()

add_subdirectory(lib)
add_subdirectory(src)
