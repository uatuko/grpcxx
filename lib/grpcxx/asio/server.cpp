#include "server.h"

#ifdef BOOST_ASIO_STANDALONE
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#else
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <asio/write.hpp>
#endif

#include "conn.h"

namespace grpcxx {
namespace asio {
ASIO_NS::awaitable<void> server::conn(ASIO_NS::ip::tcp::socket sock) {
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

ASIO_NS::awaitable<void> server::listen(std::string_view ip, int port) {
	auto executor = co_await ASIO_NS::this_coro::executor;

	ASIO_NS::ip::tcp::endpoint endpoint(ASIO_NS::ip::make_address(ip), port);
	ASIO_NS::ip::tcp::acceptor acceptor(executor, endpoint);

	for (;;) {
		auto sock =
			co_await acceptor.async_accept(ASIO_NS::make_strand(executor), ASIO_NS::use_awaitable);

		co_spawn(sock.get_executor(), conn(std::move(sock)), ASIO_NS::detached);
	}
}

void server::run(std::string_view ip, int port) {
	ASIO_NS::io_context ctx;
	co_spawn(ctx, listen(std::move(ip), std::move(port)), ASIO_NS::detached);

	ctx.run();
}
} // namespace asio
} // namespace grpcxx
