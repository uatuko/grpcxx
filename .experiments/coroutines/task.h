#pragma once

#include <coroutine>
#include <cstdio>
#include <exception>

class task {
public:
	struct promise_type;
	using coroutine_handle_type = std::coroutine_handle<promise_type>;

	struct promise_type {
		task get_return_object() { return task(coroutine_handle_type::from_promise(*this)); }

		constexpr std::suspend_never initial_suspend() const noexcept { return {}; }

		constexpr std::suspend_never final_suspend() const noexcept { return {}; }

		constexpr void return_void() const noexcept {}

		void unhandled_exception() const noexcept {
			try {
				std::rethrow_exception(std::current_exception());
			} catch (const std::exception &e) {
				std::fprintf(stderr, "Exception: %s\n", e.what());
			} catch (...) {
				std::fprintf(stderr, "Unknown exception\n");
			}
		}
	};

	explicit task(const coroutine_handle_type h) noexcept : _h(h) {}

	operator std::coroutine_handle<>() const noexcept { return _h; }

private:
	coroutine_handle_type _h;
};
