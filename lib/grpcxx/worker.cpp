#include "worker.h"

namespace grpcxx {
namespace detail {
worker::worker(int32_t id, uv_loop_t *loop, task_t &&task, fn_t &&fn) :
	_fn(std::move(fn)), _id(id), _req(new uv_work_t()), _task(std::move(task)) {
	_req->data = this;
	if (auto r = uv_queue_work(loop, _req, work_cb, after_work_cb); r != 0) {
		throw std::runtime_error(std::string("Failed to queue work: ") + uv_strerror(r));
	}
}

void worker::after_work_cb(uv_work_t *req, int status) {
	auto *w = static_cast<worker *>(req->data);
	if (status != 0 || w->_e) {
		w->_fn({w->_id, status::code_t::internal});
	} else {
		auto resp = w->_task.get_future().get();
		w->_fn(resp);
	}

	delete req;
}

void worker::run() noexcept {
	try {
		_task();
	} catch (...) {
		_e = std::current_exception();
	}
}

void worker::work_cb(uv_work_t *req) {
	auto *w = static_cast<worker *>(req->data);
	w->run();
}
} // namespace detail
} // namespace grpcxx
