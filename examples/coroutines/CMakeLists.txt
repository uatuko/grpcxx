add_executable(${PROJECT_NAME}-examples_coroutines)

target_sources(${PROJECT_NAME}-examples_coroutines
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

target_link_libraries(${PROJECT_NAME}-examples_coroutines
	PRIVATE
		libuv::uv
)
