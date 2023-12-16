add_subdirectory(coroutines)
add_subdirectory(h2c)

add_custom_target(experiments)
add_dependencies(experiments
	${PROJECT_NAME}-experiments_coroutines
	${PROJECT_NAME}-experiments_h2c-echo-server
)
