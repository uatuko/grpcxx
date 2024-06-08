#pragma once

#include <coroutine>
#include <memory>
#include <string_view>

#include <uv.h>

namespace grpcxx {
namespace uv {
namespace detail {
class writer {
public:
	writer(const writer &) = delete;
	writer(std::shared_ptr<uv_stream_t> handle, std::string_view bytes);

	bool await_ready() const noexcept;
	void await_suspend(std::coroutine_handle<> h) noexcept;
	void await_resume();

private:
	static void write_cb(uv_write_t *req, int status);

	void resume() const noexcept;

	bool                    _done;
	std::exception_ptr      _e;
	std::coroutine_handle<> _h;
	uv_write_t              _req;
};
} // namespace detail
} // namespace uv
} // namespace grpcxx
