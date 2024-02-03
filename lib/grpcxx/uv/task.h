#pragma once

#include <coroutine>
#include <cstdio>
#include <exception>
#include <functional>

namespace grpcxx {
namespace uv {
namespace detail {
class task {
public:
	using completion_fn_t = std::function<void(std::coroutine_handle<>)>;

	struct promise_type {
		task get_return_object() {
			return task(std::coroutine_handle<promise_type>::from_promise(*this));
		}

		std::suspend_always initial_suspend() const noexcept { return {}; }

		auto final_suspend() const noexcept {
			struct awaiter {
				constexpr bool await_ready() const noexcept { return false; }

				std::coroutine_handle<> await_suspend(
					std::coroutine_handle<promise_type> h) noexcept {
					auto &p = h.promise();
					if (!p._previous) {
						return std::noop_coroutine();
					}

					if (!p._fn) {
						return p._previous;
					}

					p._fn(p._previous);
					return std::noop_coroutine();
				}

				constexpr void await_resume() const noexcept {}
			};

			return awaiter{};
		}

		constexpr void return_void() const noexcept {}

		void unhandled_exception() const noexcept {
			try {
				std::rethrow_exception(std::current_exception());
			} catch (const std::exception &e) {
				std::fprintf(stderr, "[error] %s\n", e.what());
			} catch (...) {
				std::fprintf(stderr, "[error] Unknown exception\n");
			}
		}

		completion_fn_t         _fn;
		std::coroutine_handle<> _previous;
	};

	explicit task(std::coroutine_handle<promise_type> h) noexcept : _h(h) {}

	~task() { _h.destroy(); }

	auto operator co_await() noexcept {
		struct awaiter {
			bool await_ready() const noexcept { return (!_h || _h.done()); }

			std::coroutine_handle<> await_suspend(std::coroutine_handle<> coro) noexcept {
				_h.promise()._previous = coro;
				return _h;
			}

			constexpr void await_resume() const noexcept {}

			std::coroutine_handle<promise_type> _h;
		};

		return awaiter{_h};
	}

	void on_complete(completion_fn_t fn) const noexcept { _h.promise()._fn = std::move(fn); }

private:
	std::coroutine_handle<promise_type> _h;
};
} // namespace detail
} // namespace uv
} // namespace grpcxx
