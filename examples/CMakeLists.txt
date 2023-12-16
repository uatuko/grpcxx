add_subdirectory(helloworld)

add_custom_target(examples)
add_dependencies(examples
	${PROJECT_NAME}-examples_helloworld
)
