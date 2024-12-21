# NOTE: if you wonder why we use header file set BASE_DIRS
# of .., it is because other projects want to #include <grpcxx/something.h>.
# Normally the CMakeLists.txt file for building a library would be
# one level above, or the public headers into their own
# subfolder, but this works too.

add_library(grpcxx)
target_sources(grpcxx
	PRIVATE
		context.cpp
		message.cpp
		request.cpp
		server_base.cpp
	PUBLIC
		FILE_SET headers TYPE HEADERS
		BASE_DIRS ..
		FILES
			context.h
			fixed_string.h
			rpc.h
			server.h
			server_base.h
			service.h
			status.h
	PRIVATE
		FILE_SET private_headers TYPE HEADERS
		FILES
			message.h
			request.h
			response.h
)

if (NOT GRPCXX_USE_ASIO)
	target_sources(grpcxx
		PRIVATE
			uv/conn.cpp
			uv/reader.cpp
			uv/scheduler.cpp
			uv/server.cpp
			uv/writer.cpp
		PUBLIC
			FILE_SET headers TYPE HEADERS
			BASE_DIRS ..
			FILES
				uv/server.h
				uv/scheduler.h
				uv/task.h
		PRIVATE
			FILE_SET private_headers TYPE HEADERS
			FILES
				uv/conn.h
				uv/coroutine.h
				uv/reader.h
				uv/writer.h
	)
else()
	target_sources(grpcxx
		PRIVATE
			asio/conn.cpp
			asio/server.cpp
		PUBLIC
			FILE_SET headers TYPE HEADERS
			BASE_DIRS ..
			FILES
				asio/server.h
		PRIVATE
			FILE_SET private_headers TYPE HEADERS
			FILES
				asio/conn.h
	)
endif()

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
)

if (NOT GRPCXX_USE_ASIO)
	target_link_libraries(grpcxx
		PUBLIC
			libuv::uv
	)

    # On Mac OS X, when libc++ is used with CLang,
    # stop_token for some reason is still not stabilized.
    # Therefore we enable experimental features.
	target_compile_options(grpcxx
        PUBLIC 
            $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-fexperimental-library>
	)

else() # ASIO version
	target_link_libraries(grpcxx
		PUBLIC
			asio
	)

	target_compile_definitions(grpcxx
		PUBLIC
			GRPCXX_USE_ASIO
	)
endif()

include(GNUInstallDirs)
install(TARGETS grpcxx EXPORT grpcxx
		LIBRARY COMPONENT Runtime
        FILE_SET headers
			COMPONENT Development)
