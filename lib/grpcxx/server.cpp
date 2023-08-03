#include "server.h"

#include "h2/conn.h"

#include "status.h"

namespace grpcxx {
server::server() {
	// TODO: error handling
	uv_loop_init(&_loop);
	uv_tcp_init(&_loop, &_handle);

	_handle.data = this;
}

void server::alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	buf->base = new char[suggested_size]();
	buf->len  = suggested_size;
}

void server::close_cb(uv_handle_t *handle) {
	if (handle->data != nullptr) {
		auto *conn = static_cast<h2::conn *>(handle->data);
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

	h2::conn::event_cb_t event_cb =
		std::bind(&server::event, static_cast<class server *>(server->data), std::placeholders::_1);

	h2::conn::write_cb_t write_cb = std::bind(
		&server::write,
		static_cast<class server *>(server->data),
		reinterpret_cast<uv_stream_t *>(handle),
		std::placeholders::_1,
		std::placeholders::_2);

	auto *conn = new h2::conn(std::move(event_cb), std::move(write_cb));

	handle->data = conn;
	uv_read_start(reinterpret_cast<uv_stream_t *>(handle), alloc_cb, read_cb);
}

void server::event(const h2::event &ev) {
	// FIXME: use `h2::event::type_t::stream_headers` events to stop processing early
	switch (ev.type) {
	case h2::event::type_t::stream_end: {
		// FIXME: only for debugging
		{
			for (const auto &[name, value] : ev.stream->headers) {
				std::printf("  %s: %s\n", name.c_str(), value.c_str());
			}

			std::printf("[data] size: %zu\n  %s\n", ev.stream->data.size(), ev.stream->data.data());
		}

		auto it = ev.stream->headers.find(":path");
		if (it == ev.stream->headers.end()) {
			send(ev.stream, {status::code_t::not_found});
			break;
		}

		// :path = /service.name/MethodName
		// FIXME: validate path
		auto svc_name    = it->second.substr(1, it->second.find("/", 1) - 1);
		auto method_name = it->second.substr(svc_name.size() + 2);

		auto svc_it = _services.find(svc_name);
		if (svc_it == _services.end()) {
			send(ev.stream, {status::code_t::not_found});
			break;
		}

		// FIXME: validate length-prefixed message data
		const auto &raw_data = ev.stream->data;
		uint8_t     encoded  = raw_data[0];
		uint32_t    size =
			(raw_data[1] << 24) | (raw_data[2] << 16) | (raw_data[3] << 8) | (raw_data[4]);

		try {
			auto result = svc_it->second(method_name, raw_data.substr(5));

			// FIXME: actually send response data
			send(ev.stream, result.first, result.second);
		} catch (std::exception &e) {
			// FIXME: handle errors
			std::fprintf(stderr, "server::listen(), error: %s\n", e.what());
			send(ev.stream, {status::code_t::internal});
		}

		break;
	}

	default:
		std::printf("server::listen(), event: %hhu\n", ev.type);
		break;
	}
}

void server::read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
	auto *conn = static_cast<h2::conn *>(stream->data);
	if (nread < 0) {
		if (nread != UV_EOF) {
			// FIXME: handle errors
			std::fprintf(stderr, "server::read_cb(), error: %s\n", uv_err_name(nread));
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

void server::send(std::shared_ptr<h2::stream> stream, const status &s, const data_t &msg) {
	// Headers
	{
		stream->send({
			{":status", "200"},
			{"content-type", "application/grpc"},
		});
	}

	// Length-prefixed message
	if (msg.size() > 0) {
		data_t data;
		data.reserve(5 + msg.size());

		std::array<data_t::value_type, 5> bytes;
		bytes[0] = 0x00;
		bytes[1] = (static_cast<uint32_t>(msg.size()) >> 24) & 0xff;
		bytes[2] = (static_cast<uint32_t>(msg.size()) >> 16) & 0xff;
		bytes[3] = (static_cast<uint32_t>(msg.size()) >> 8) & 0xff;
		bytes[4] = static_cast<uint32_t>(msg.size()) & 0xff;

		data.append(bytes.data(), bytes.size());
		data.append(msg);

		stream->send(data);
	}

	// Trailers
	stream->send(
		{
			{"grpc-status", s},
		},
		true);
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
		std::fprintf(stderr, "server::write(), error: %s\n", uv_strerror(r));

		delete[] buf;
		delete req;
		return 0;
	}

	return size;
}

void server::write_cb(uv_write_t *req, int status) {
	if (status) {
		// FIXME: handle errors
		std::fprintf(stderr, "server::write_cb(), error: %s\n", uv_strerror(status));
	}

	auto *buf = static_cast<char *>(req->data);
	delete[] buf;

	delete req;
}
} // namespace grpcxx
