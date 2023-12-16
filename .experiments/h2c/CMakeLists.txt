add_executable(${PROJECT_NAME}-experiments_h2c-echo-server
	echo-server.cpp echo-server.h
)

target_link_libraries(${PROJECT_NAME}-experiments_h2c-echo-server
	PRIVATE
		libnghttp2::nghttp2
		libuv::uv
)
