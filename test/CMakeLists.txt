include(GoogleTest)

add_executable(grpcxx-unit-tests)
target_sources(grpcxx-unit-tests
    PRIVATE
        sanity_check.cpp
)

if(NOT GRPCXX_USE_ASIO)
    # libuv-related test sources
    target_sources(grpcxx-unit-tests
        PRIVATE
            uv/server.cpp
    )
endif()

target_link_libraries(grpcxx-unit-tests
    PRIVATE GTest::gtest_main
            grpcxx
            fmt::fmt)

gtest_discover_tests(grpcxx-unit-tests
    XML_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
