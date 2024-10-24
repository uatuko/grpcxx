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

target_compile_definitions(protoc-gen-grpcxx
	PRIVATE
		PLUGIN_NAME="$<TARGET_NAME:protoc-gen-grpcxx>"
		PLUGIN_VERSION="${PROJECT_VERSION}"
)

install(TARGETS protoc-gen-grpcxx
        EXPORT grpcxx
		COMPONENT Development)
