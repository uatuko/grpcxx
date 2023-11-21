cmake_minimum_required(VERSION 3.23)
project(grpcxx VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

include(cmake/dependencies.cmake)

add_subdirectory(examples)
add_subdirectory(lib)
add_subdirectory(src)
