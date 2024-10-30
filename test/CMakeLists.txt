include(GoogleTest)

add_executable(grpcxx-unit-tests)
target_sources(grpcxx-unit-tests
    PRIVATE
        sanity_check.cpp)
target_link_libraries(grpcxx-unit-tests
    PRIVATE GTest::gtest_main)

gtest_discover_tests(grpcxx-unit-tests
    XML_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
