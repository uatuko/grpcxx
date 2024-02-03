#pragma once

#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include "../server_base.h"

namespace grpcxx {
namespace asio {
class server : public ::grpcxx::detail::server_base {
public:
	server(const server &) = delete;
	server(::asio::io_context &ctx) noexcept;

	void run(std::string_view ip, int port);

private:
	::asio::awaitable<void> conn(::asio::ip::tcp::socket sock);

	::asio::io_context &_ctx;
};
} // namespace asio
} // namespace grpcxx
