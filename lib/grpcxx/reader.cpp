#include "reader.h"

#include <string>

namespace grpcxx {
namespace detail {
reader::reader(stream_t stream) : _eos(false), _nread(0), _stream(stream) {
	_stream->data = this;
}

reader::~reader() noexcept {
	uv_read_stop(_stream.get());
}

void reader::alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	auto *r = static_cast<reader *>(handle->data);
	*buf    = uv_buf_init(r->_buf.data(), r->_buf.capacity());
}

bool reader::await_ready() noexcept {
	if (auto r = uv_read_start(_stream.get(), alloc_cb, read_cb); r != 0) {
		_e = std::make_exception_ptr(
			std::runtime_error(std::string("Failed to start reading data: ") + uv_strerror(r)));

		return true;
	}

	return false;
}

std::string_view reader::await_resume() {
	_h = nullptr;

	if (_e) {
		std::rethrow_exception(_e);
	}

	if (_eos) {
		return {};
	}

	size_t size = _nread;
	_nread      = 0;

	uv_read_stop(_stream.get());
	return {_buf.data(), size};
}

void reader::await_suspend(std::coroutine_handle<> h) noexcept {
	_h = h;
}

void reader::read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
	auto *r = static_cast<reader *>(stream->data);

	if (nread >= 0) {
		r->_nread = nread;
	} else {
		if (nread == UV_EOF) {
			r->_eos = true;
		} else {
			r->_e = std::make_exception_ptr(
				std::runtime_error(std::string("Failed to read data: ") + uv_strerror(nread)));
		}
	}

	r->resume();
}

void reader::resume() const noexcept {
	if (_h) {
		_h();
	}
}
} // namespace detail
} // namespace grpcxx
