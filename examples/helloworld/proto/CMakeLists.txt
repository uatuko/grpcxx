# helloworld-proto
cmake_path(SET greeter_proto  ${CMAKE_CURRENT_SOURCE_DIR}/helloworld/v1/greeter.proto)
cmake_path(SET greeter_header ${CMAKE_CURRENT_BINARY_DIR}/helloworld/v1/greeter.pb.h)
cmake_path(SET greeter_source ${CMAKE_CURRENT_BINARY_DIR}/helloworld/v1/greeter.pb.cc)

set(protos
	${greeter_proto}
)

set(headers
	${greeter_header}
)

set(sources
	${greeter_source}
)

add_custom_command(
	OUTPUT  ${headers} ${sources}
	DEPENDS ${protos}
	COMMAND ${Protobuf_PROTOC_EXECUTABLE}
	ARGS
		--proto_path=${CMAKE_CURRENT_SOURCE_DIR}
		--proto_path=${Protobuf_INCLUDE_DIR}
		--cpp_out=${CMAKE_CURRENT_BINARY_DIR}
		${protos}
)

add_library(helloworld-proto
	${sources}
)

target_include_directories(helloworld-proto
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(helloworld-proto
	protobuf::libprotobuf
)
