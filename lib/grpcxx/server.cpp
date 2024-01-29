#include "server.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/strand.hpp>
#include <asio/write.hpp>

#include "conn.h"
#include "request.h"
#include "response.h"

namespace grpcxx {
server::server(std::size_t n) noexcept : _ctx(n), _services() {}

asio::awaitable<void> server::conn(asio::ip::tcp::socket sock) {
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

detail::response server::process(const detail::request &req) const noexcept {
	if (!req) {
		return {req.id(), status::code_t::invalid_argument};
	}

	auto it = _services.find(req.service());
	if (it == _services.end()) {
		return {req.id(), status::code_t::not_found};
	}

	context          ctx(req);
	detail::response resp(req.id());
	try {
		auto r = it->second(ctx, req.method(), req.data());
		resp.status(std::move(r.first));
		resp.data(std::move(r.second));
	} catch (std::exception &e) {
		return {req.id(), status::code_t::internal};
	}

	return resp;
}

void server::run(const std::string_view ip, int port) {
	co_spawn(
		_ctx,
		[this, &ip, &port]() -> asio::awaitable<void> {
			auto executor = co_await asio::this_coro::executor;

			asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip), port);
			asio::ip::tcp::acceptor acceptor(executor, endpoint);

			for (;;) {
				auto sock = co_await acceptor.async_accept(
					asio::make_strand(executor), asio::use_awaitable);

				co_spawn(sock.get_executor(), conn(std::move(sock)), asio::detached);
			}
		}(),
		asio::detached);

	_ctx.run();
}
} // namespace grpcxx
