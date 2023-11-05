#pragma once

#include <coroutine>
#include <forward_list>
#include <unordered_map>

#include <uv.h>

#include "h2/session.h"

#include "request.h"
#include "response.h"

namespace grpcxx {
namespace detail {
class conn {
public:
	using requests_t = std::forward_list<request>;
	using streams_t  = std::unordered_map<int32_t, request>;

	conn()             = default;
	conn(const conn &) = delete;

	constexpr bool await_ready() const noexcept { return _end; }
	void           await_suspend(std::coroutine_handle<> h) noexcept { _h = h; }
	void           await_resume() noexcept { _h = nullptr; }

	void end() noexcept;
	void read(size_t n) noexcept;
	void write(uv_stream_t *stream) noexcept;
	void write(uv_stream_t *stream, response resp) noexcept;

	requests_t reqs() noexcept;

	static void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
	static void close_cb(uv_handle_t *handle);

private:
	template <std::size_t N> class buffer_t {
	public:
		constexpr char       *data() noexcept { return &_data[0]; }
		constexpr std::size_t capacity() const noexcept { return N; }

	private:
		char _data[N];
	};

	static void write_cb(uv_write_t *req, int status);

	bool                    _end = false;
	std::coroutine_handle<> _h;

	requests_t _reqs;
	streams_t  _streams;

	buffer_t<1024> _buf; // FIXME: make size configurable
	h2::session    _session;
};
} // namespace detail
} // namespace grpcxx
