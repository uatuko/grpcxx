#include "grpcxx/uv/server.h"
#include "gtest/gtest.h"

#include "ping_pong.grpcxx.pb.h"
#include "uv.h"

#include <fmt/core.h>
#include <sys/socket.h>

#include <cstring>
#include <stop_token>
#include <thread>

template <>
auto PingPong::ServiceImpl::call<PingPong::rpchello>(grpcxx::context &, const Ping &req)
	-> rpchello::result_type {
	Pong res;
	return {grpcxx::status::code_t::ok, res};
}

namespace /* anonymous */ {

struct fd_t {
	fd_t(int fd) : _fd(fd) {}
	fd_t(const fd_t &) = delete;
	fd_t(fd_t &&that) : _fd(that.release()) {}
	~fd_t() { close(_fd); }

	int release() {
		using std::swap;
		int ret = -1;
		swap(ret, _fd);
		return ret;
	}
	operator int() const { return _fd; }

	int _fd;
};

TEST(UvServer, loop_cleanup) {
	grpcxx::uv::server    server(1);
	PingPong::ServiceImpl ping_pong;
	PingPong::Service     service{ping_pong};
	server.add(service);

	auto *loop = server.listen("127.0.0.1", 0);
	std::jthread{
		[&server](std::stop_token token) { server.run(std::move(token)); }
	};

	int count = 0;
	uv_walk(
		loop,
		[](uv_handle_t *handle, void *arg) {
			int *count = static_cast<int *>(arg);
			(*count)++;

			if (!uv_is_closing(handle)) {
				// Only the async handler should be active at this point, which will be closed when
				// the server is destroyed
				ASSERT_EQ(UV_ASYNC, handle->type);
			}
		},
		&count
	);

	ASSERT_EQ(3, count); // UV_ASYNC (1), UV_TCP(12), UV_TIMER (13)
}

TEST(UvServer, run_with_address_and_stop) {
	grpcxx::uv::server    server{1};
	PingPong::ServiceImpl ping_pong;
	PingPong::Service     service{ping_pong};
	server.add(service);

	auto thr = std::jthread{
		[&server](std::stop_token stop_token) { server.run("::1", 0, std::move(stop_token)); }};

	// TODO: we should do an exchange here, as soon as grpcxx also provides
	// a client. So far this was tested manually via Postman.
}

TEST(UvServer, run_with_existing_fd_and_stop) {
	sockaddr_in addr;
	socklen_t   addr_len{sizeof(addr)};

	auto sockfd = fd_t{socket(AF_INET, SOCK_STREAM, 0)};
	ASSERT_GE(sockfd, 0);

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port        = htons(0); // 0 means any available port

	ASSERT_GE(0, bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)));
	auto port = ntohs(addr.sin_port);

	grpcxx::uv::server    server{1};
	PingPong::ServiceImpl ping_pong;
	PingPong::Service     service{ping_pong};
	server.add(service);

	auto fd  = sockfd.release(); // the server will take ownership, and it will close it.
	auto thr = std::jthread{
		[&server, fd](std::stop_token stop_token) { server.run(fd, std::move(stop_token)); }};

	// TODO: we should do an exchange here, as soon as grpcxx also provides
	// a client. So far this was tested manually via Postman.
}

TEST(UvServer, run_with_external_loop) {
	grpcxx::uv::server    server(1);
	PingPong::ServiceImpl ping_pong;
	PingPong::Service     service{ping_pong};
	server.add(service);

	uv_loop_t *uv_loop = server.listen("127.0.0.1", 0);

	// TODO: we should do an exchange here, as soon as grpcxx also provides
	// a client. So far this was tested manually via Postman
	// (and UV_RUN_DEFAULT / UV_RUN_ONCE).

	// Here we manually run the loop
	uv_run(uv_loop, UV_RUN_NOWAIT);

	// Loop ownership is with the server, which will do the cleanup
}
} // namespace
