cmake_minimum_required(VERSION 3.23)
project(grpcxx VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

include(CMakeDependentOption)
cmake_dependent_option(GRPCXX_BUILD_EXAMPLES
	"Build examples when this is the root project" ON
	"CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
)

cmake_dependent_option(GRPCXX_BUILD_EXPERIMENTS
	"Build experiments when this is the root project" ON
	"CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
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
