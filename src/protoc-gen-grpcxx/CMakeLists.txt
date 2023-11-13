add_executable(protoc-gen-grpcxx)
target_sources(protoc-gen-grpcxx
	PRIVATE
		grpcxx.cpp
		main.cpp
	PRIVATE
		FILE_SET private_headers TYPE HEADERS
		FILES
			grpcxx.h
)

target_link_libraries(protoc-gen-grpcxx
	PRIVATE
		fmt::fmt
		protobuf::libprotobuf
		protobuf::libprotoc
)
