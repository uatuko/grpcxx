add_executable(${PROJECT_NAME}-experiments_coroutines)

target_sources(${PROJECT_NAME}-experiments_coroutines
	PRIVATE
		main.cpp
	PRIVATE
		FILE_SET private_headers TYPE HEADERS
		FILES
			conn.h
			reader.h
			server.h
			task.h
			writer.h
)

target_link_libraries(${PROJECT_NAME}-experiments_coroutines
	PRIVATE
		libuv::uv
)
