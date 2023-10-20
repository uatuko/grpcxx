#include "conn.h"

#include <string>

namespace grpcxx {
namespace detail {
conn::conn(uv_stream_t *stream) : _handle(new uv_tcp_t{}, deleter{}) {
	uv_tcp_init(stream->loop, _handle.get());
	_handle->data = this;

	if (auto r = uv_accept(stream, reinterpret_cast<uv_stream_t *>(_handle.get())); r != 0) {
		throw std::runtime_error(std::string("Failed to accept connection: ") + uv_strerror(r));
	}

	if (auto r = uv_read_start(reinterpret_cast<uv_stream_t *>(_handle.get()), alloc_cb, read_cb);
		r != 0) {
		throw std::runtime_error(std::string("Failed to start reading data: ") + uv_strerror(r));
	}
}

void conn::alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	auto *c = static_cast<conn *>(handle->data);
	*buf    = uv_buf_init(c->_buf.data(), c->_buf.capacity());
}

void conn::read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
	auto *c = static_cast<conn *>(stream->data);
	if (nread <= 0) {
		return;
	}

	c->_session.recv(reinterpret_cast<const uint8_t *>(buf->base), nread);
}
} // namespace detail
} // namespace grpcxx
