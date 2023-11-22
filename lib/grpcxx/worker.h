#pragma once

#include <condition_variable>
#include <coroutine>
#include <queue>
#include <thread>

#include <uv.h>

namespace grpcxx {
namespace detail {
class worker {
public:
	worker();
	worker(const worker &) = delete;

	uv_loop_t *loop() noexcept { return &_loop; }

	void enqueue(std::coroutine_handle<> h) noexcept;
	void run();

private:
	using handles_t = std::queue<std::coroutine_handle<>>;

	std::condition_variable _cv;
	handles_t               _handles;
	std::mutex              _mutex;

	uv_loop_t _loop;
};
} // namespace detail
} // namespace grpcxx
