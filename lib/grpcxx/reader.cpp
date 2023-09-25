#include "reader.h"

namespace grpcxx {
namespace detail {
void reader::alloc(uv_buf_t *buf) noexcept {
	_size = 0;
	*buf  = uv_buf_init(_buf.data(), _buf.capacity());
}

bool reader::await_ready() const noexcept {
	return _size > 0;
}

std::string_view reader::await_resume() noexcept {
	_h = nullptr;

	std::string_view data(_buf.data(), _size);
	_size = 0;

	return data;
}

void reader::await_suspend(std::coroutine_handle<> h) noexcept {
	_h = h;
}

void reader::close() noexcept {
	_eos = true;
	resume();
}

void reader::resume() const noexcept {
	if (_h) {
		_h();
	}
}
} // namespace detail
} // namespace grpcxx
