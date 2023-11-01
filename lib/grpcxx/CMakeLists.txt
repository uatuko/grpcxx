add_library(grpcxx)
target_sources(grpcxx
	PRIVATE
		conn.cpp
		message.cpp
		reader.cpp
		request.cpp
		server.cpp
		worker.cpp
		writer.cpp
	PUBLIC
		FILE_SET headers TYPE HEADERS
		FILES
			fixed_string.h
			rpc.h
			server.h
			service.h
			status.h
	PRIVATE
		FILE_SET private_headers TYPE HEADERS
		FILES
			conn.h
			coroutine.h
			reader.h
			request.h
			response.h
			message.h
			worker.h
			writer.h
)

# h2 sources
target_sources(grpcxx
	PRIVATE
		h2/session.cpp
	PRIVATE
		FILE_SET private_headers TYPE HEADERS
		FILES
			h2/event.h
			h2/headers.h
			h2/session.h
)

target_link_libraries(grpcxx
	PRIVATE
		libnghttp2::nghttp2
	PUBLIC
		libuv::uv
)
