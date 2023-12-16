#include "server.h"

#include <unistd.h>

#include "conn.h"
#include "coroutine.h"
#include "request.h"
#include "response.h"

namespace grpcxx {
server::server(std::size_t n) noexcept : _handle(), _loop(), _pool(n), _services() {
	uv_loop_init(&_loop);
	uv_tcp_init(&_loop, &_handle);

	_handle.data = this;
}

detail::coroutine server::accept(uv_stream_t *stream) {
	// Due to limitations on Windows, libuv doesn't support moving a handler from one loop to
	// another. On *nix systems this can be done by duplicating the socket.
	// Ref: https://github.com/libuv/libuv/issues/390
	uv_tcp_t tmp_handle;
	uv_tcp_init(&_loop, &tmp_handle);

	if (auto r = uv_accept(stream, reinterpret_cast<uv_stream_t *>(&tmp_handle)); r != 0) {
		throw std::runtime_error(std::string("Failed to accept connection: ") + uv_strerror(r));
	}

	uv_os_fd_t tmp_fd;
	uv_fileno(reinterpret_cast<uv_handle_t *>(&tmp_handle), &tmp_fd);

	auto fd = dup(tmp_fd);
	uv_close(reinterpret_cast<uv_handle_t *>(&tmp_handle), nullptr);

	auto *loop = co_await _pool.schedule();
	{ // Executed in a worker thread
		uv_tcp_t handle;
		uv_tcp_init(loop, &handle);
		uv_tcp_open(&handle, fd);

		detail::conn c(&handle);
		while (c) {
			for (const auto &req : co_await c) {
				if (!c) {
					break;
				}

				auto resp = process(req);
				c.write(resp);
			}
		}
	}
}

void server::conn_cb(uv_stream_t *stream, int status) {
	if (status < 0) {
		return;
	}

	auto *s = static_cast<server *>(stream->data);
	s->accept(stream);
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

void server::run(const std::string_view &ip, int port) {
	// TODO: error handling
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0);
	uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb);

	uv_run(&_loop, UV_RUN_DEFAULT);
}
} // namespace grpcxx
