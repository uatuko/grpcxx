#include "server.h"

#include "conn.h"
#include "coroutine.h"

#include <stdexcept>
#include <stop_token>
#include <string>

namespace grpcxx {
namespace uv {
server::server(std::size_t n) noexcept : _loop{}, _scheduler{_loop, n} {
	uv_tcp_init(_loop, &_handle);
	_handle.data = this;
}

server::server(uv_loop_t &loop, std::size_t n) noexcept : _loop{loop}, _scheduler{_loop, n} {
	uv_tcp_init(_loop, &_handle);
	_handle.data = this;
}

server::~server() noexcept {
	if (uv_loop_alive(_loop)) {
		uv_stop(_loop);
	}
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

void server::prepare(std::string_view ip, int port) {
	sockaddr_storage addr;
	if (uv_ip4_addr(ip.data(), port, reinterpret_cast<sockaddr_in *>(&addr)) != 0 &&
		uv_ip6_addr(ip.data(), port, reinterpret_cast<sockaddr_in6 *>(&addr)) != 0) {
		throw std::runtime_error(std::string(ip) + " is not a valid IPv4 or IPv6 address");
	}

	if (auto r = uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0); r != 0) {
		throw std::runtime_error(std::string("Failed to bind to tcp address: ") + uv_strerror(r));
	}

	start_listening();
}

void server::prepare(uv_os_sock_t sock) {
	if (auto r = uv_tcp_open(&_handle, sock); r != 0) {
		throw std::runtime_error(
			std::string("Failed to open socket as a tcp handle: ") + uv_strerror(r));
	}

#ifndef WIN32
	// libuv windows implementation already checks the socket address family and calls
	// uv_tcp_getsockname() within uv_tcp_open(), no need to check again.
	sockaddr_storage name;
	int              namelen = sizeof(name);
	if (auto r = uv_tcp_getsockname(&_handle, (sockaddr *)&name, &namelen); r != 0) {
		throw std::runtime_error(
			std::string("Failed to retrieve bound address for socket: ") + uv_strerror(r));
	}
#endif

	start_listening();
}

void server::start_listening() {
	if (auto r = uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), TCP_LISTEN_BACKLOG, conn_cb);
		r != 0) {
		throw std::runtime_error(
			std::string("Failed to listen for connections: ") + uv_strerror(r));
	}
}

void server::run(std::string_view ip, int port, std::stop_token stop_token) {
	prepare(std::move(ip), port);
	setup_stop_timer(std::move(stop_token));
	uv_run(_loop, UV_RUN_DEFAULT);
}

void server::run(uv_os_sock_t sock, std::stop_token stop_token) {
	prepare(sock);
	setup_stop_timer(std::move(stop_token));
	uv_run(_loop, UV_RUN_DEFAULT);
}

void server::setup_stop_timer(std::stop_token stop_token) noexcept {
	// See https://docs.libuv.org/en/v1.x/guide/eventloops.html#stopping-an-event-loop
	// for a rationale around the shutdown timer.

	_stop_token = std::move(stop_token);
	uv_timer_init(_loop, &_check_stop_timer);
	_check_stop_timer.data = this;
	uv_timer_start(
		&_check_stop_timer,
		[](uv_timer_t *handle) {
			auto &self = *static_cast<server *>(handle->data);
			if (self._stop_token.stop_requested()) {
				uv_timer_stop(handle);
				uv_close(reinterpret_cast<uv_handle_t *>(&self._handle), nullptr);
				if (self._loop.is_managed()) {
					uv_stop(handle->loop);
				}
			}
		},
		SHUTDOWN_CHECK_INTERVAL_MS,
		SHUTDOWN_CHECK_INTERVAL_MS);
}

} // namespace uv
} // namespace grpcxx
