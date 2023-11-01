#pragma once

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

	void alloc(uv_buf_t *buf) noexcept;
	void read(size_t n) noexcept;
	void write(uv_stream_t *stream) noexcept;
	void write(uv_stream_t *stream, response resp) noexcept;

	requests_t reqs() noexcept;

private:
	template <std::size_t N> class buffer_t {
	public:
		constexpr char       *data() noexcept { return &_data[0]; }
		constexpr std::size_t capacity() const noexcept { return N; }

	private:
		char _data[N];
	};

	static void write_cb(uv_write_t *req, int status);

	requests_t _reqs;
	streams_t  _streams;

	buffer_t<1024> _buf; // FIXME: make size configurable
	h2::session    _session;
};
} // namespace detail
} // namespace grpcxx
