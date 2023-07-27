#include "server.h"

#include "h2/conn.h"

namespace grpcxx {
server::server() {
	// TODO: error handling
	uv_loop_init(&_loop);
	uv_tcp_init(&_loop, &_handle);
}

void server::alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	buf->base = new char[suggested_size]();
	buf->len  = suggested_size;
}

void server::close_cb(uv_handle_t *handle) {
	if (handle->data != nullptr) {
		auto *conn = reinterpret_cast<h2::conn *>(handle->data);
		delete conn;
	}

	delete handle;
}

void server::conn_cb(uv_stream_t *server, int status) {
	auto *handle = new uv_tcp_t();
	uv_tcp_init(server->loop, handle);

	if (uv_accept(server, reinterpret_cast<uv_stream_t *>(handle)) != 0) {
		// FIXME: handle errors
		uv_close(reinterpret_cast<uv_handle_t *>(handle), close_cb);
		return;
	}

	h2::conn::listener_t listener = std::bind(
		&server::listen, reinterpret_cast<class server *>(server->data), std::placeholders::_1);

	h2::conn::writer_t writer = std::bind(
		&server::write,
		reinterpret_cast<class server *>(server->data),
		reinterpret_cast<uv_stream_t *>(handle),
		std::placeholders::_1,
		std::placeholders::_2);

	auto *conn = new h2::conn(std::move(listener), std::move(writer));

	handle->data = conn;
	uv_read_start(reinterpret_cast<uv_stream_t *>(handle), alloc_cb, read_cb);
}

void server::listen(const h2::event &ev) {
	switch (ev.type) {
	case h2::event_type::data:
		std::printf("[data] size: %zu\n  %s\n", ev.stream->data.size(), ev.stream->data.data());

		break;

	case h2::event_type::headers: {
		for (const auto &[name, value] : ev.stream->headers) {
			std::printf("  %s: %s\n", name.c_str(), value.c_str());
		}

		break;
	}

	default:
		break;
	}
}

void server::read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
	auto *conn = reinterpret_cast<h2::conn *>(stream->data);
	if (nread < 0) {
		if (nread != UV_EOF) {
			// FIXME: handle errors
			std::fprintf(stderr, "Read error: %s\n", uv_err_name(nread));
		}

		uv_close(reinterpret_cast<uv_handle_t *>(stream), close_cb);
		delete[] buf->base;
		return;
	}

	conn->recv(reinterpret_cast<const uint8_t *>(buf->base), nread);
	conn->send();
}

void server::run(const std::string_view &ip, int port) {
	// TODO: error handling
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0);
	uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb);

	uv_run(&_loop, UV_RUN_DEFAULT);
}

size_t server::write(uv_stream_t *handle, const uint8_t *data, size_t size) {
	auto *req = new uv_write_t();
	auto *buf = new char[size]();

	std::memcpy(buf, data, size);
	req->data = buf;

	auto uv_buf = uv_buf_init(buf, size);
	auto r      = uv_write(req, handle, &uv_buf, 1, write_cb);
	if (r != 0) {
		// FIXME: handle errors
		std::fprintf(stderr, "uv_write() error: %s\n", uv_strerror(r));

		delete[] buf;
		delete req;
		return 0;
	}

	return size;
}

void server::write_cb(uv_write_t *req, int status) {
	if (status) {
		// FIXME: handle errors
		std::fprintf(stderr, "Write error: %s\n", uv_strerror(status));
	}

	auto *buf = reinterpret_cast<char *>(req->data);
	delete[] buf;

	delete req;
}
} // namespace grpcxx
