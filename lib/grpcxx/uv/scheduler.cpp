#include "scheduler.h"

namespace grpcxx {
namespace uv {
namespace detail {
scheduler::scheduler(uv_loop_t *loop, std::size_t n) {
	if (n == 0) {
		return;
	}

	_async.data = this;
	uv_async_init(loop, &_async, [](uv_async_t *handle) {
		auto *s = static_cast<scheduler *>(handle->data);
		s->join();
	});

	for (; n > 0; n--) {
		_threads.emplace_back([this] { this->loop(); });
	}
}

scheduler::~scheduler() {
	_stop = true;
	for (auto &t : _threads) {
		if (t.joinable()) {
			t.join();
		}
	}
}

bool scheduler::enqueue(std::coroutine_handle<> h) noexcept {
	if (_threads.empty()) {
		return false;
	}

	{
		std::lock_guard lock(_shifters_mutex);
		_shifters.push(std::move(h));
		_threads_cv.notify_one();
	}
	return true;
}

void scheduler::join() {
	while (!_joiners.empty()) {
		std::coroutine_handle<> h;
		{
			std::lock_guard lock(_joiners_mutex);
			h = std::move(_joiners.front());
			_joiners.pop();
		}

		h();
	}
}

void scheduler::loop() {
	while (!_stop) {
		std::unique_lock lock(_shifters_mutex);
		while (!_stop && _shifters.size() == 0) {
			_threads_cv.wait_for(lock, std::chrono::microseconds(100));
		}

		if (_stop) {
			break;
		}

		auto h = std::move(_shifters.front());
		_shifters.pop();
		lock.unlock();

		h();
	}
}

task scheduler::run(fn_t fn) {
	auto t = wrap(std::move(fn));
	if (_threads.empty()) {
		return t;
	}

	t.on_complete([&](std::coroutine_handle<> h) {
		{
			std::lock_guard lock(_joiners_mutex);
			_joiners.push(std::move(h));
		}

		uv_async_send(&_async);
	});

	return t;
}

task scheduler::wrap(fn_t fn) {
	co_await shift();
	fn();
}
} // namespace detail
} // namespace uv
} // namespace grpcxx
