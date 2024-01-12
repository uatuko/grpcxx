#include "worker.h"

namespace grpcxx {
namespace detail {
worker::worker() : _async(), _handles(), _loop(), _mutex() {
	uv_loop_init(&_loop);
}

void worker::enqueue(std::coroutine_handle<> h) noexcept {
	{
		std::lock_guard lock(_mutex);
		_handles.emplace(h);
	}

	uv_async_send(&_async);
}

void worker::run() {
	_async.data = this;
	uv_async_init(&_loop, &_async, [](uv_async_t *handle) {
		auto *w = static_cast<worker *>(handle->data);

		while (!w->_handles.empty()) {
			handles_t::value_type h;
			{
				std::lock_guard lock(w->_mutex);
				h = w->_handles.front();
				w->_handles.pop();
			}

			h.resume();
		}
	});

	uv_run(&_loop, UV_RUN_DEFAULT);
}
} // namespace detail
} // namespace grpcxx
