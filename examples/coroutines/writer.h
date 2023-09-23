#pragma once

#include <coroutine>
#include <memory>
#include <string>

#include <uv.h>

class writer {
public:
	writer(const writer &) = delete;

	writer(std::shared_ptr<uv_stream_t> handle, std::string_view data) :
		_done(false), _e(nullptr), _h(nullptr), _req() {
		_req.data = this;

		auto buf = uv_buf_init(const_cast<char *>(data.data()), data.size());
		if (auto r = uv_write(&_req, handle.get(), &buf, 1, write_cb); r != 0) {
			throw std::runtime_error(
				std::string("Failed to start writing data: ") + uv_strerror(r));
		}
	}

	bool await_ready() const noexcept { return _done; }

	void await_suspend(std::coroutine_handle<> h) noexcept { _h = h; }

	void await_resume() {
		_h = nullptr;

		if (_e) {
			std::rethrow_exception(_e);
		}
	}

	void resume() const noexcept {
		if (_h) {
			_h();
		}
	}

private:
	void done() noexcept {
		_done = true;
		resume();
	}

	void exception(const std::exception_ptr &e) noexcept { _e = e; }

	static void write_cb(uv_write_t *req, int status) {
		auto *w = static_cast<writer *>(req->data);
		if (status != 0) {
			auto e = std::make_exception_ptr(
				std::runtime_error(std::string("Failed to write data: ") + uv_strerror(status)));
			w->exception(e);
		}

		w->done();
	}

	bool                    _done;
	std::exception_ptr      _e;
	std::coroutine_handle<> _h;
	uv_write_t              _req;
};
