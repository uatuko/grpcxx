#include "server.h"

#include "conn.h"
#include "coroutine.h"

namespace grpcxx {
namespace uv {
namespace detail {
server::server(std::size_t n) noexcept : _scheduler(_loop, n) {
	uv_tcp_init(_loop, &_handle);
	_handle.data = this;
}

detail::coroutine server::conn(uv_stream_t *stream) {
	detail::conn c(stream);
	auto         reader = c.reader();
	while (reader) {
		auto bytes = co_await reader;
		if (bytes.empty()) {
			continue;
		}

		co_await _scheduler.run([&] {
			for (auto &req : c.read(bytes)) {
				auto resp = process(req);
				c.write(std::move(resp));
			}
		});

		co_await c.flush();
	}
}

void server::conn_cb(uv_stream_t *stream, int status) {
	if (status < 0) {
		return;
	}

	auto *s = static_cast<server *>(stream->data);
	s->conn(stream);
}

void server::run(std::string_view ip, int port) {
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	if (auto r = uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0); r != 0) {
		throw std::runtime_error(std::string("Failed to bind to tcp address: ") + uv_strerror(r));
	}

	if (auto r = uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb); r != 0) {
		throw std::runtime_error(
			std::string("Failed to listen for connections: ") + uv_strerror(r));
	}

	uv_run(_loop, UV_RUN_DEFAULT);
}
} // namespace detail
} // namespace uv
} // namespace grpcxx
