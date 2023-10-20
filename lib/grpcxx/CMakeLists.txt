add_library(grpcxx)
target_sources(grpcxx
	PRIVATE
		conn.cpp
		server.cpp
		writer.cpp
	PUBLIC
		FILE_SET headers TYPE HEADERS
		FILES
			fixed_string.h
			rpc.h
			server.h
			service.h
			status.h
)

# h2 sources
target_sources(grpcxx
	PRIVATE
		h2/session.cpp
	PRIVATE
		FILE_SET private_headers TYPE HEADERS
		FILES
			conn.h
			task.h
			writer.h
			h2/event.h
			h2/session.h
)

target_link_libraries(grpcxx
	PRIVATE
		libnghttp2::nghttp2
	PUBLIC
		libuv::uv
)
