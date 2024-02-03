#include "writer.h"

#include <string>

namespace grpcxx {
namespace uv {
namespace detail {
writer::writer(std::shared_ptr<uv_stream_t> handle, std::string_view bytes) :
	_done(false), _e(nullptr), _h(nullptr), _req() {
	_req.data = this;

	auto buf = uv_buf_init(const_cast<char *>(bytes.data()), bytes.size());
	if (auto r = uv_write(&_req, handle.get(), &buf, 1, write_cb); r != 0) {
		throw std::runtime_error(std::string("Failed to start writing data: ") + uv_strerror(r));
	}
}

bool writer::await_ready() const noexcept {
	return _done;
}

void writer::await_resume() {
	_h = nullptr;

	if (_e) {
		std::rethrow_exception(_e);
	}
}

void writer::await_suspend(std::coroutine_handle<> h) noexcept {
	_h = h;
}

void writer::resume() const noexcept {
	if (_h) {
		_h();
	}
}

void writer::write_cb(uv_write_t *req, int status) {
	auto *w = static_cast<writer *>(req->data);
	if (status != 0) {
		w->_e = std::make_exception_ptr(
			std::runtime_error(std::string("Failed to write data: ") + uv_strerror(status)));
	}

	w->_done = true;
	w->resume();
}
} // namespace detail
} // namespace uv
} // namespace grpcxx
