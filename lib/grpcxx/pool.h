#pragma once

#include <forward_list>
#include <thread>
#include <vector>

#include <uv.h>

#include "worker.h"

namespace grpcxx {
namespace detail {
class pool {
public:
	struct awaiter {
	public:
		awaiter(pool *p) : _loop(nullptr), _pool(p) {}

		constexpr bool await_ready() const noexcept { return false; }
		uv_loop_t     *await_resume() const noexcept { return _loop; }

		void await_suspend(std::coroutine_handle<> h) noexcept {
			auto &w = _pool->worker();
			w.enqueue(h);

			_loop = w.loop();
		}

	private:
		uv_loop_t *_loop;
		pool      *_pool;
	};

	pool(std::size_t n);
	pool(const pool &) = delete;

	worker &worker() noexcept;

	awaiter schedule() noexcept;

private:
	using threads_t = std::forward_list<std::thread>;
	using workers_t = std::vector<detail::worker>;

	std::size_t _idx;
	threads_t   _threads;
	workers_t   _workers;
};
} // namespace detail
} // namespace grpcxx
