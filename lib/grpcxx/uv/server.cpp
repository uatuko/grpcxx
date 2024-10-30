#include "server.h"

#include "conn.h"
#include "coroutine.h"
#include "uv.h"

#include <sys/socket.h>

#include <stdexcept>
#include <stop_token>
#include <string>

namespace grpcxx {
namespace uv {
server::server(std::size_t n) noexcept : _scheduler(_loop, n) {
	uv_tcp_init(_loop, &_handle);
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

void server::run(std::string_view ip, int port, std::stop_token stop_token) {
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	if (auto r = uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0); r != 0) {
		throw std::runtime_error(std::string("Failed to bind to tcp address: ") + uv_strerror(r));
	}

	run(_handle, std::move(stop_token));
}

void server::run(int fd, std::stop_token stop_token) {
	if (auto r = uv_tcp_open(&_handle, fd); r != 0) {
		throw std::runtime_error(
			std::string("Provided fd ") + std::to_string(fd) +
			" is not a bound network socket: " + uv_strerror(r));
	}

	run(_handle, std::move(stop_token));
}

void server::run(uv_tcp_t &handle, std::stop_token stop_token) {
	if (auto r = uv_listen(reinterpret_cast<uv_stream_t *>(&handle), TCP_LISTEN_BACKLOG, conn_cb);
		r != 0) {
		throw std::runtime_error(
			std::string("Failed to listen for connections: ") + uv_strerror(r));
	}

	// See https://docs.libuv.org/en/v1.x/guide/eventloops.html#stopping-an-event-loop
	// for a rationale around the shutdown timer.

	uv_timer_init(_loop, &_check_stop_timer);
	_check_stop_timer.data = &stop_token;
	uv_timer_start(
		&_check_stop_timer,
		[](uv_timer_t *handle) {
			const auto stop_token = static_cast<const std::stop_token *>(handle->data);
			if (stop_token->stop_requested()) {
				uv_timer_stop(handle);
				uv_stop(handle->loop);
			}
		},
		SHUTDOWN_CHECK_INTERVAL_MS,
		SHUTDOWN_CHECK_INTERVAL_MS);

	handle.data = this;
	uv_run(_loop, UV_RUN_DEFAULT);
}

server::loop_t::loop_t() {
	uv_loop_init(&_loop);
}

server::loop_t::~loop_t() noexcept {
	// Ensures all remaining handles are cleaned up (but without any
	// special close callback)
	uv_walk(
		&_loop,
		[](uv_handle_t *handle, void *) {
			if (!uv_is_closing(handle)) {
				uv_close(handle, nullptr);
			}
		},
		nullptr);

	while (uv_loop_close(&_loop) == UV_EBUSY) {
		uv_run(&_loop, UV_RUN_ONCE);
	}
}

} // namespace uv
} // namespace grpcxx
