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

TEST(UvServer, run_in_nested_loop) {
	class server : public ::grpcxx::uv::server {
	public:
		server() : ::grpcxx::uv::server{1} { prepare("127.0.0.1", 0); }

		void process_pending() { ::grpcxx::uv::server::process_pending(); }
	};

	server                server;
	PingPong::ServiceImpl ping_pong;
	PingPong::Service     service{ping_pong};
	server.add(service);

	// Integration with other event loops, such as the one of libevent,
	// is possible. Here we just nest uv loops to avoid introducing
	// additional dependencies and show how this can be achieved without
	// the need of moving the server to additional threads.
	uv_loop_t outer_loop;
	uv_loop_init(&outer_loop);

	uv_timer_t inner_loop_timer;
	uv_timer_init(&outer_loop, &inner_loop_timer);
	inner_loop_timer.data = &server;
	uv_timer_start(
		&inner_loop_timer,
		[](uv_timer_t *handle) {
			auto server = static_cast<class server *>(handle->data);
			server->process_pending();
		},
		0,
		0);
	uv_run(&outer_loop, UV_RUN_DEFAULT);

	// TODO: we should do an exchange here, as soon as grpcxx also provides
	// a client. So far this was tested manually via Postman (with a non-zero
	// repeat value for the timer).

	// Cleanup
	uv_close(reinterpret_cast<uv_handle_t *>(&inner_loop_timer), nullptr);
	uv_run(&outer_loop, UV_RUN_DEFAULT);
	uv_loop_close(&outer_loop);
}

} // namespace
