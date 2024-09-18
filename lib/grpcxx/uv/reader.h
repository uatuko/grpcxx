#pragma once

#include <uv.h>

#include <coroutine>
#include <cstdio>
#include <memory>
#include <string_view>

namespace grpcxx {
namespace uv {
namespace detail {
class reader {
public:
	using stream_t = std::shared_ptr<uv_stream_t>;

	reader(const reader &) = delete;
	reader(stream_t stream);

	~reader() noexcept;

	constexpr operator bool() const noexcept { return !(_eos || _e); }

	bool             await_ready() noexcept;
	void             await_suspend(std::coroutine_handle<> h) noexcept;
	std::string_view await_resume();

private:
	template <std::size_t N> class buffer_t {
	public:
		constexpr char       *data() noexcept { return &_data[0]; }
		constexpr std::size_t capacity() const noexcept { return N; }

	private:
		char _data[N];
	};

	static void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
	static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);

	void resume() const noexcept;

	buffer_t<1024>          _buf; // FIXME: make size configurable
	std::exception_ptr      _e;
	bool                    _eos;
	std::coroutine_handle<> _h;
	ssize_t                 _nread;
	stream_t                _stream;
};
} // namespace detail
} // namespace uv
} // namespace grpcxx
