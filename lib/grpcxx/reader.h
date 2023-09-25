#pragma once

#include <coroutine>
#include <string_view>

#include <uv.h>

namespace grpcxx {
namespace detail {
class reader {
public:
	reader()               = default;
	reader(const reader &) = delete;

	operator bool() const noexcept { return !_eos; }

	std::size_t size() const noexcept { return _size; }

	void size(std::size_t s) noexcept {
		if (s > _buf.capacity()) {
			s = _buf.capacity();
		}

		_size = s;
		resume();
	}

	bool             await_ready() const noexcept;
	void             await_suspend(std::coroutine_handle<> h) noexcept;
	std::string_view await_resume() noexcept;

	void alloc(uv_buf_t *buf) noexcept;
	void close() noexcept;

private:
	template <std::size_t N> class buffer_t {
	public:
		constexpr char       *data() noexcept { return &_data[0]; }
		constexpr std::size_t capacity() const noexcept { return N; }

	private:
		char _data[N];
	};

	void resume() const noexcept;

	buffer_t<1024>          _buf;
	bool                    _eos = false;
	std::coroutine_handle<> _h;
	std::size_t             _size;
};
} // namespace detail
} // namespace grpcxx
