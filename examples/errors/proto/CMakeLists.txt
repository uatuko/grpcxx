# errors-proto
add_library(errors-proto)
target_sources(errors-proto
	PRIVATE
		examples/v1/errors.proto
		google/rpc/code.proto
		google/rpc/status.proto
)

target_include_directories(errors-proto
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_link_libraries(errors-proto
	protobuf::libprotobuf
)

protobuf_generate(TARGET errors-proto)
protobuf_generate(TARGET errors-proto
	LANGUAGE grpcxx
	GENERATE_EXTENSIONS .grpcxx.pb.h
	PLUGIN "protoc-gen-grpcxx=\$<TARGET_FILE:libgrpcxx::protoc-gen>"
)
