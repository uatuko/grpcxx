add_subdirectory(errors)
add_subdirectory(helloworld)

add_custom_target(examples)
add_dependencies(examples
	${PROJECT_NAME}-examples_errors
	${PROJECT_NAME}-examples_helloworld
)
