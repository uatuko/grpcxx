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

	conn(const conn &) = delete;
	conn(uv_tcp_t *handle) noexcept;

	operator bool() const noexcept { return !_eos; }

	bool       await_ready() const noexcept { return _eos || !_reqs.empty(); }
	void       await_suspend(std::coroutine_handle<> h) noexcept { _h = h; }
	requests_t await_resume() noexcept;

	void read(std::size_t n);
	void write(response resp) noexcept;

private:
	template <std::size_t N> class buffer_t {
	public:
		constexpr char       *data() noexcept { return &_data[0]; }
		constexpr std::size_t capacity() const noexcept { return N; }

	private:
		char _data[N];
	};

	static void close_cb(uv_handle_t *handle);
	static void write_cb(uv_write_t *req, int status);

	void resume() noexcept;
	void write() noexcept;

	std::coroutine_handle<> _h;

	buffer_t<1024> _buf; // FIXME: make size configurable
	bool           _eos;
	requests_t     _reqs;
	h2::session    _session;
	streams_t      _streams;

	uv_tcp_t *_handle;
};
} // namespace detail
} // namespace grpcxx
