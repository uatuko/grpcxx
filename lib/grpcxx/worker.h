#pragma once

#include <future>

#include <uv.h>

#include "response.h"

namespace grpcxx {
namespace detail {
class worker {
public:
	using fn_t   = std::function<void(response)>;
	using task_t = std::packaged_task<response()>;

	worker(const worker &) = delete;
	worker(int32_t id, uv_loop_t *loop, task_t &&task, fn_t &&fn);

private:
	static void work_cb(uv_work_t *req);
	static void after_work_cb(uv_work_t *req, int status);

	void run() noexcept;

	int32_t _id;

	std::exception_ptr _e;
	fn_t               _fn;
	task_t             _task;

	uv_work_t *_req;
};
} // namespace detail
} // namespace grpcxx
