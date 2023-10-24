#pragma once

#include <coroutine>
#include <memory>

#include <uv.h>

#include "h2/session.h"

namespace grpcxx {
namespace detail {
class writer {
public:
	using session_t = std::shared_ptr<h2::session>;
	using stream_t  = std::shared_ptr<uv_stream_t>;

	writer(const writer &) = delete;
	writer(stream_t handle, session_t session);

	bool await_ready() const noexcept;
	void await_suspend(std::coroutine_handle<> h) noexcept;
	void await_resume();

private:
	static void write_cb(uv_write_t *req, int status);

	void resume() noexcept;

	bool                    _done;
	std::exception_ptr      _e;
	std::coroutine_handle<> _h;
	stream_t                _handle;
	uv_write_t              _req;
	session_t               _session;
};
} // namespace detail
} // namespace grpcxx
