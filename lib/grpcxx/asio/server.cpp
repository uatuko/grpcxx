#include "server.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/strand.hpp>
#include <asio/write.hpp>

#include "conn.h"

namespace grpcxx {
namespace asio {
server::server(::asio::io_context &ctx) noexcept : _ctx(ctx) {}

::asio::awaitable<void> server::conn(::asio::ip::tcp::socket sock) {
	detail::conn c(std::move(sock));
	while (c) {
		for (const auto &req : co_await c.reqs()) {
			if (!c) {
				break;
			}

			auto resp = process(req);
			co_await c.write(std::move(resp));
		}
	}
}

void server::run(std::string_view ip, int port) {
	co_spawn(
		_ctx,
		[this, &ip, &port]() -> ::asio::awaitable<void> {
			auto executor = co_await ::asio::this_coro::executor;

			::asio::ip::tcp::endpoint endpoint(::asio::ip::make_address(ip), port);
			::asio::ip::tcp::acceptor acceptor(executor, endpoint);

			for (;;) {
				auto sock = co_await acceptor.async_accept(
					::asio::make_strand(executor), ::asio::use_awaitable);

				co_spawn(sock.get_executor(), conn(std::move(sock)), ::asio::detached);
			}
		}(),
		::asio::detached);

	_ctx.run();
}
} // namespace asio
} // namespace grpcxx
