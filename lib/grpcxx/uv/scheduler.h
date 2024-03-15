#pragma once

#include <condition_variable>
#include <coroutine>
#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <thread>

#include <uv.h>

#include "task.h"

namespace grpcxx {
namespace uv {
namespace detail {
class scheduler {
public:
	using fn_t = std::function<void()>;

	scheduler(const scheduler &) = delete;
	scheduler(uv_loop_t *loop, std::size_t n);
	~scheduler();

	task run(fn_t fn);

	auto shift() noexcept {
		struct awaiter {
			constexpr bool await_ready() const noexcept { return false; }
			constexpr void await_resume() const noexcept {}

			bool await_suspend(std::coroutine_handle<> h) const noexcept {
				return _scheduler->enqueue(h);
			}

			scheduler *_scheduler;
		};

		return awaiter{this};
	}

private:
	using handles_t = std::queue<std::coroutine_handle<>>;
	using threads_t = std::list<std::thread>;

	bool enqueue(std::coroutine_handle<> h) noexcept;
	void join();
	void loop();
	task wrap(fn_t fn);

	bool                    _stop = false;
	threads_t               _threads;
	std::condition_variable _threads_cv;

	handles_t  _shifters;
	std::mutex _shifters_mutex;

	uv_async_t _async;
	handles_t  _joiners;
	std::mutex _joiners_mutex;
};
} // namespace detail
} // namespace uv
} // namespace grpcxx
