add_subdirectory(proto)

add_executable(${PROJECT_NAME}-examples_helloworld
	main.cpp
)

target_link_libraries(${PROJECT_NAME}-examples_helloworld
	PRIVATE
		helloworld-proto
		libgrpcxx::grpcxx
)
