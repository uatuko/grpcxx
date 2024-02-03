add_subdirectory(proto)

add_executable(${PROJECT_NAME}-examples_errors
	main.cpp
)

target_link_libraries(${PROJECT_NAME}-examples_errors
	PRIVATE
		errors-proto
		libgrpcxx::grpcxx
)

if (GRPCXX_USE_ASIO)
	target_compile_definitions(${PROJECT_NAME}-examples_errors
		PUBLIC
			GRPCXX_USE_ASIO
	)
endif()
