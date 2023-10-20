#include "server.h"

#include <future>

#include "conn.h"
#include "task.h"

namespace grpcxx {
server::server() {
	// TODO: error handling
	uv_loop_init(&_loop);
	uv_tcp_init(&_loop, &_handle);

	_handle.data = this;
}

detail::task server::conn(uv_stream_t *stream) {
	std::printf("[info] connection - start\n");

	detail::conn c(stream);

	while (c) {
		auto ev = co_await c.session();

		switch (ev.type) {
		case h2::event::type_t::session_write:
			co_await c.write(ev.data);
			break;

		case h2::event::type_t::stream_header:
			std::printf(
				"  [header](%d) %s : %s\n",
				ev.stream_id.value(),
				ev.header->name.c_str(),
				ev.header->value.c_str());
			break;

		default:
			std::printf("event: %hhu\n", ev.type);
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
