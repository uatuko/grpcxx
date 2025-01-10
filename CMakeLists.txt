cmake_minimum_required(VERSION 3.27)
project(grpcxx VERSION 0.6.1 LANGUAGES CXX)

cmake_policy(SET CMP0135 NEW) # CMake 3.24
cmake_policy(SET CMP0144 NEW) # CMake 3.27

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

option(GRPCXX_HERMETIC_BUILD "Fetch and build all dependencies instead of relying on system libraries" ON)
option(GRPCXX_USE_ASIO "Use asio instead of libuv for I/O and event loop" OFF)

include(CMakeDependentOption)
cmake_dependent_option(GRPCXX_BUILD_EXAMPLES
	"Build examples when this is the root project" ON
	"CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
)

cmake_dependent_option(GRPCXX_BUILD_EXPERIMENTS
	"Build experiments when this is the root project and not using asio" ON
	"NOT GRPCXX_USE_ASIO; CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
)

cmake_dependent_option(GRPCXX_BUILD_TESTING
	"Build tests when BUILD_TESTING flag is set or this is the root project" ON
	"BUILD_TESTING OR CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF
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

if(GRPCXX_BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()

# Installation steps to make find_package(grpcxx) work
# in other projects.
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
set(cmake_conf_dir ${CMAKE_INSTALL_LIBDIR}/cmake/grpcxx)
configure_package_config_file(cmake/grpcxx-config.cmake.in cmake/grpcxx-config.cmake
    INSTALL_DESTINATION ${cmake_conf_dir})
write_basic_package_version_file(cmake/grpcxx-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion)
install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/cmake/grpcxx-version.cmake
			${CMAKE_CURRENT_BINARY_DIR}/cmake/grpcxx-config.cmake
        DESTINATION ${cmake_conf_dir}
		COMPONENT Development)
export(EXPORT grpcxx
       FILE ${CMAKE_CURRENT_BINARY_DIR}/cmake/grpcxx-targets.cmake
       NAMESPACE grpcxx::)
install(EXPORT grpcxx
        NAMESPACE grpcxx::
        COMPONENT Development
		FILE grpcxx-targets.cmake
        DESTINATION ${cmake_conf_dir})
