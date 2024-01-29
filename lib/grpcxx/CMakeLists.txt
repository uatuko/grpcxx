add_library(grpcxx)
target_sources(grpcxx
	PRIVATE
		conn.cpp
		context.cpp
		message.cpp
		request.cpp
		server.cpp
	PUBLIC
		FILE_SET headers TYPE HEADERS
		FILES
			context.h
			fixed_string.h
			rpc.h
			server.h
			service.h
			status.h
	PRIVATE
		FILE_SET private_headers TYPE HEADERS
		FILES
			conn.h
			message.h
			request.h
			response.h
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
		asio
)
