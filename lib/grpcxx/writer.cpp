#include "writer.h"

#include <string>

namespace grpcxx {
namespace detail {
writer::writer(stream_t handle, session_t session) :
	_done(false), _e(nullptr), _h(nullptr), _handle(handle), _req{}, _session(session) {
	_req.data = this;

	auto data = _session->pending();
	if (data.size() == 0) {
		_done = true;
		return;
	}

	auto buf = uv_buf_init(const_cast<char *>(data.data()), data.size());
	if (auto r = uv_write(&_req, _handle.get(), &buf, 1, write_cb); r != 0) {
		throw std::runtime_error(std::string("Failed to start writing data: ") + uv_strerror(r));
	}
}

bool writer::await_ready() const noexcept {
	return (_e || _done);
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

void writer::resume() noexcept {
	if (_session) {
		if (auto data = _session->pending(); data.size() > 0) {
			_req = uv_write_t{
				.data = this,
			};

			auto buf = uv_buf_init(const_cast<char *>(data.data()), data.size());
			auto r   = uv_write(&_req, _handle.get(), &buf, 1, write_cb);
			if (r == 0) {
				return;
			}

			auto e = std::make_exception_ptr(
				std::runtime_error(std::string("Failed to write data: ") + uv_strerror(r)));
			_e = e;
		}
	}

	_done = true;
	if (_h) {
		_h();
	}
}

void writer::write_cb(uv_write_t *req, int status) {
	auto *w = static_cast<writer *>(req->data);
	if (status != 0) {
		auto e = std::make_exception_ptr(
			std::runtime_error(std::string("Failed to write data: ") + uv_strerror(status)));
		w->_e = e;
	}

	w->resume();
}
} // namespace detail
} // namespace grpcxx
