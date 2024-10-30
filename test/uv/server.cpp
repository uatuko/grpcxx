#include "grpcxx/uv/server.h"

#include "gtest/gtest.h"

#include <fmt/core.h>
#include <sys/socket.h>

#include <cstring>
#include <stop_token>
#include <thread>

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

TEST(UvServer, RunWithAddressAndStop) {
	grpcxx::uv::server server{1};
	auto               thr = std::jthread{[&server](std::stop_token stop_token) {
        server.run("127.0.0.1", 0, std::move(stop_token));
    }};
}

TEST(UvServer, RunWithExistingFdAndStop) {
	sockaddr_in addr;
	socklen_t   addr_len{sizeof(addr)};

	auto sockfd = fd_t{socket(AF_INET, SOCK_STREAM, 0)};
	ASSERT_GE(sockfd, 0);

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port        = htons(0); // 0 means any available port

	ASSERT_GE(0, bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)));

	grpcxx::uv::server server{1};
	auto fd  = sockfd.release(); // the server will take ownership, and it will close it.
	auto thr = std::jthread{
		[&server, fd](std::stop_token stop_token) { server.run(fd, std::move(stop_token)); }};
}

} // namespace
