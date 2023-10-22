#include "server.h"

#include <future>
#include <optional>
#include <unordered_map>

#include "conn.h"
#include "request.h"
#include "task.h"

namespace grpcxx {
using requests_t = std::unordered_map<int32_t, std::optional<detail::request>>;

server::server() {
	// TODO: error handling
	uv_loop_init(&_loop);
	uv_tcp_init(&_loop, &_handle);

	_handle.data = this;
}

detail::task server::conn(uv_stream_t *stream) {
	std::printf("[info] connection - start\n");

	detail::conn c(stream);
	requests_t   requests;

	auto &session = c.session();

	while (session) {
		auto ev = co_await session;

		if (ev.stream_id.value_or(0) != 0 && !requests.contains(ev.stream_id.value())) {
			requests.insert({
				ev.stream_id.value(),
				{ev.stream_id.value()},
			});
		}

		switch (ev.type) {
		case h2::event::type_t::session_error: {
			std::printf("  [error] %s\n", ev.error.value().c_str());
			break;
		}

		case h2::event::type_t::session_write:
			co_await c.write(ev.data);
			break;

		case h2::event::type_t::stream_data: {
			auto &req = requests[ev.stream_id.value()];
			req->read(ev.data);
			break;
		}

		case h2::event::type_t::stream_header: {
			auto &req = requests[ev.stream_id.value()];
			req->header(ev.header->name, ev.header->value);
			break;
		}

		default:
			std::printf(
				"  [event] stream_id: %d, type: %hhu\n", ev.stream_id.value_or(-1), ev.type);
			break;
		}
	}

	std::printf("[info] connection - end\n");
}

void server::conn_cb(uv_stream_t *stream, int status) {
	if (status < 0) {
		return;
	}

	auto *s = static_cast<server *>(stream->data);
	s->conn(stream);
}

void server::run(const std::string_view &ip, int port) {
	// TODO: error handling
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0);
	uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb);

	uv_run(&_loop, UV_RUN_DEFAULT);
}
} // namespace grpcxx
