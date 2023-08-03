add_subdirectory(h2c)
add_subdirectory(helloworld)

add_custom_target(examples)
add_dependencies(examples
	${PROJECT_NAME}-examples_h2c-echo-server
	${PROJECT_NAME}-examples_helloworld
)
