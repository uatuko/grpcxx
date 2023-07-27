add_subdirectory(grpcxx)

add_library(libgrpcxx INTERFACE)
target_link_libraries(libgrpcxx
	INTERFACE grpcxx
)

target_include_directories(libgrpcxx
	INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_library(libgrpcxx::grpcxx ALIAS libgrpcxx)
