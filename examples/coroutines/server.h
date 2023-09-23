#pragma once

#include <cstdio>
#include <future>
#include <string>

#include <uv.h>

#include "conn.h"
#include "task.h"

class server {
public:
	server() {
		uv_loop_init(&_loop);
		uv_tcp_init(&_loop, &_handle);

		_handle.data = this;
	}

	task accept(uv_stream_t *stream) {
		std::printf("[info] connection - start\n");
		conn c(stream);

		while (c) {
			auto data = co_await c.read();
			std::printf("[debug] incoming data, size: %zu bytes\n", data.size());

			if (data.size() > 0) {
				auto out =
					std::async([&data]() -> std::string { return "reply: " + std::string(data); });

				co_await c.write(out.get());
			}
		}

		std::printf("[info] connection - end\n");
	}

	void run(const std::string_view &ip, int port) {
		struct sockaddr_in addr;
		uv_ip4_addr(ip.data(), port, &addr);

		if (auto r = uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0); r != 0) {
			throw std::runtime_error(
				std::string("Failed to bind to tcp address: ") + uv_strerror(r));
		}

		if (auto r = uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb); r != 0) {
			throw std::runtime_error(
				std::string("Failed to listen for connections: ") + uv_strerror(r));
		}

		uv_run(&_loop, UV_RUN_DEFAULT);
	}

private:
	static void conn_cb(uv_stream_t *stream, int status) {
		if (status < 0) {
			std::fprintf(
				stderr, "[error] Unexpected status for new connection: %s\n", uv_strerror(status));
			return;
		}

		auto *s = static_cast<server *>(stream->data);
		s->accept(stream);
	}

	uv_tcp_t  _handle;
	uv_loop_t _loop;
};
