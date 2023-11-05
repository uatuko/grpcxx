#include "worker.h"

namespace grpcxx {
namespace detail {
worker::worker() : _cv(), _handles(), _loop(), _mutex() {
	uv_loop_init(&_loop);
}

void worker::enqueue(std::coroutine_handle<> h) noexcept {
	std::lock_guard lock(_mutex);
	_handles.emplace(h);
	_cv.notify_one();
}

void worker::run() {
	while (true) {
		if (uv_run(&_loop, UV_RUN_ONCE) != 0) {
			// Give extra cpu time if the loop is "active"
			for (uint8_t r = _loop.active_handles + 1; r > 0; r--) {
				if (uv_run(&_loop, UV_RUN_NOWAIT) == 0) {
					break;
				}
			}
		}

		std::unique_lock lock(_mutex);
		while (uv_loop_alive(&_loop) == 0 && _handles.empty()) {
			// FIXME: make timeout configurable
			_cv.wait_for(lock, std::chrono::microseconds(100));
		}

		while (!_handles.empty()) {
			auto &h = _handles.front();
			_handles.pop();

			lock.unlock();
			h.resume();
			lock.lock();
		}
	}
}
} // namespace detail
} // namespace grpcxx
