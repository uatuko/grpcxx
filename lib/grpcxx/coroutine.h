#pragma once

#include <coroutine>
#include <cstdio>
#include <exception>

namespace grpcxx {
namespace detail {
struct coroutine {
	struct promise_type {
		constexpr coroutine get_return_object() const noexcept { return {}; }

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
};
} // namespace detail
} // namespace grpcxx
