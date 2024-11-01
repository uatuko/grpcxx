include(GoogleTest)

add_executable(grpcxx-unit-tests)
target_sources(grpcxx-unit-tests
    PRIVATE
        test_sanity_check.cpp
)

set(proto_files
    proto/ping_pong.proto)

foreach(proto IN LISTS proto_files)
    cmake_path(REMOVE_EXTENSION proto 
        LAST_ONLY
        OUTPUT_VARIABLE stem)
    set(stem ${CMAKE_CURRENT_BINARY_DIR}/${stem})

    cmake_path(REMOVE_FILENAME proto 
        OUTPUT_VARIABLE dir)
    set(destdir "${CMAKE_CURRENT_BINARY_DIR}/${dir}")
    
    file(MAKE_DIRECTORY ${destdir})

    add_custom_command(
        OUTPUT ${stem}.grpcxx.pb.h
               ${stem}.pb.h
               ${stem}.pb.cc
        COMMAND protobuf::protoc
                    "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto"
                    "--cpp_out=${destdir}"
                    "--grpcxx_out=${destdir}"
                    "--plugin=$<TARGET_FILE:protoc-gen-grpcxx>"
                    ${CMAKE_CURRENT_SOURCE_DIR}/${proto}
        COMMENT "Generating sources from ${proto}"
        VERBATIM
    )

    target_sources(grpcxx-unit-tests
        PRIVATE ${stem}.pb.cc)
endforeach()

target_include_directories(grpcxx-unit-tests
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/proto)

if(NOT GRPCXX_USE_ASIO)
    # libuv-related test sources
    target_sources(grpcxx-unit-tests
        PRIVATE
            uv/test_server.cpp
    )
endif()

target_link_libraries(grpcxx-unit-tests
    PRIVATE GTest::gtest_main
            protobuf::libprotobuf
            grpcxx
            fmt::fmt)

gtest_discover_tests(grpcxx-unit-tests
    XML_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
