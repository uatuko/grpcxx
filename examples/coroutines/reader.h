#pragma once

#include <coroutine>
#include <memory>
#include <string_view>

class reader {
public:
	template <std::size_t N> class buffer_t {
	public:
		buffer_t()                 = default;
		buffer_t(const buffer_t &) = delete;

		constexpr void clear() noexcept { _size = 0; }

		constexpr char       *data() noexcept { return &_data[0]; }
		constexpr std::size_t capacity() const noexcept { return N; }

		const std::size_t size() const noexcept { return _size; }

		constexpr void size(std::size_t s) noexcept {
			if (s > capacity()) {
				_size = capacity();
				return;
			}

			_size = s;
		}

	private:
		char        _data[N];
		std::size_t _size;
	};

	reader()               = default;
	reader(const reader &) = delete;

	constexpr operator bool() const noexcept { return !_eos; }

	bool await_ready() const noexcept { return _eos; }

	void await_suspend(std::coroutine_handle<> h) { _h = h; }

	std::string_view await_resume() {
		_h = nullptr;
		return data();
	}

	auto            &buffer() noexcept { return _buf; }
	std::string_view data() noexcept { return {_buf.data(), _buf.size()}; }

	void close() noexcept {
		_eos = true;
		resume();
	}

	void resume() const noexcept {
		if (_h) {
			_h();
		}
	}

private:
	buffer_t<1024>          _buf;
	bool                    _eos = false;
	std::coroutine_handle<> _h;
};
