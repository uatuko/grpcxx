#pragma once

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>

#include "../server_base.h"

namespace grpcxx {
namespace asio {
class server : public ::grpcxx::detail::server_base {
public:
	server(const server &) = delete;
	server()               = default;

	::asio::awaitable<void> listen(std::string_view ip, int port);
	void                    run(std::string_view ip, int port);

private:
	::asio::awaitable<void> conn(::asio::ip::tcp::socket sock);
};
} // namespace asio
} // namespace grpcxx
