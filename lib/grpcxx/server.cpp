#include "server.h"

#include <list>

#include <unistd.h>

#include "conn.h"
#include "coroutine.h"
#include "request.h"
#include "response.h"

uv_async_t _async;

std::mutex                          _mutex;
std::queue<std::coroutine_handle<>> _handles;

class task {
public:
	class promise_type {
	public:
		task get_return_object() {
			return task(std::coroutine_handle<promise_type>::from_promise(*this));
		}

		std::suspend_always initial_suspend() const noexcept { return {}; }

		auto final_suspend() const noexcept {
			struct awaiter {
				constexpr bool await_ready() const noexcept { return false; }

				void await_suspend(std::coroutine_handle<promise_type> h) noexcept {
					{
						std::lock_guard lock(_mutex);
						_handles.push(h.promise()._continuation);
					}
					uv_async_send(&_async);
				}

				constexpr void await_resume() const noexcept {}
			};

			return awaiter();
		}

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

		void continuation(std::coroutine_handle<> continuation) noexcept {
			_continuation = continuation;
		}

	private:
		std::coroutine_handle<> _continuation = std::noop_coroutine();
	};

	explicit task(std::coroutine_handle<promise_type> h) noexcept : _h(h) {}

	~task() {
		// Final suspend never resumes, so it's safe to destroy without checking
		_h.destroy();
	}

	auto operator co_await() noexcept {
		struct awaiter {
			bool await_ready() const noexcept { return (!_h || _h.done()); }

			std::coroutine_handle<> await_suspend(std::coroutine_handle<> coro) noexcept {
				_h.promise().continuation(coro);
				return _h;
			}

			constexpr void await_resume() const noexcept {}

			std::coroutine_handle<promise_type> _h;
		};

		return awaiter{_h};
	}

private:
	std::coroutine_handle<promise_type> _h;
};

class threadpool {
public:
	explicit threadpool(const std::size_t n) {
		for (std::size_t i = 0; i < n; ++i) {
			std::thread t([this]() { this->loop(); });
			_threads.push_back(std::move(t));
		}
	}

	~threadpool() { shutdown(); }

	auto schedule() {
		struct awaiter {
			threadpool *_threadpool;

			constexpr bool await_ready() const noexcept { return false; }
			constexpr void await_resume() const noexcept {}

			void await_suspend(std::coroutine_handle<> coro) const noexcept {
				_threadpool->enqueue(coro);
			}
		};

		return awaiter{this};
	}

private:
	std::list<std::thread> _threads;

	std::mutex                          _mutex;
	std::condition_variable             _cv;
	std::queue<std::coroutine_handle<>> _coros;

	bool _stop = false;

	void loop() {
		while (!_stop) {
			std::unique_lock lock(_mutex);
			while (!_stop && _coros.size() == 0) {
				_cv.wait_for(lock, std::chrono::microseconds(100));
			}

			if (_stop) {
				break;
			}

			auto coro = std::move(_coros.front());
			_coros.pop();
			lock.unlock();

			coro.resume();
		}
	}

	void enqueue(std::coroutine_handle<> coro) noexcept {
		std::unique_lock lock(_mutex);
		_coros.push(std::move(coro));
		_cv.notify_one();
	}

	void shutdown() {
		_stop = true;
		while (_threads.size() > 0) {
			auto &t = _threads.back();
			if (t.joinable()) {
				t.join();
			}

			_threads.pop_back();
		}
	}
};

std::hash<std::thread::id> hasher;
threadpool                 pool(10);

namespace grpcxx {
server::server(std::size_t n) noexcept : _handle(), _loop(), _pool(n), _services() {
	uv_loop_init(&_loop);
	uv_tcp_init(&_loop, &_handle);

	_handle.data = this;

	uv_async_init(&_loop, &_async, [](uv_async_t *) {
		while (!_handles.empty()) {
			std::coroutine_handle<> h;
			{
				std::lock_guard lock(_mutex);
				h = std::move(_handles.front());
				_handles.pop();
			}

			h();
		}
	});
}

detail::coroutine server::conn(uv_stream_t *stream) {
	std::string buf;
	buf.reserve(1024);

	detail::conn c(stream);
	auto         reader  = c.reader();
	auto        &session = c.session();
	while (reader) {
		auto bytes = co_await reader;
		if (bytes.empty()) {
			continue;
		}

		for (auto chunk = session.pending(); chunk.size() > 0; chunk = session.pending()) {
			buf.append(chunk);
		}

		for (auto &req : c.read(bytes)) {
			auto resp = process(req);
			session.headers(
				resp.id(),
				{
					{":status", "200"},
					{"content-type", "application/grpc"},
				});

			session.data(resp.id(), resp.bytes());
			for (auto chunk = session.pending(); chunk.size() > 0; chunk = session.pending()) {
				buf.append(chunk);
			}

			const auto &status = resp.status();
			session.trailers(
				resp.id(),
				{
					{"grpc-status", status},
					{"grpc-status-details-bin", status.details()},
				});

			for (auto chunk = session.pending(); chunk.size() > 0; chunk = session.pending()) {
				buf.append(chunk);
			}
		}

		if (!buf.empty()) {
			co_await c.write(buf);
			buf.clear();
		}
	}
}

void server::conn_cb(uv_stream_t *stream, int status) {
	if (status < 0) {
		return;
	}

	auto *s = static_cast<server *>(stream->data);
	s->conn(stream);
}

detail::response server::process(const detail::request &req) const noexcept {
	if (!req) {
		return {req.id(), status::code_t::invalid_argument};
	}

	auto it = _services.find(req.service());
	if (it == _services.end()) {
		return {req.id(), status::code_t::not_found};
	}

	context          ctx(req);
	detail::response resp(req.id());
	try {
		auto r = it->second(ctx, req.method(), req.data());
		resp.status(std::move(r.first));
		resp.data(std::move(r.second));
	} catch (std::exception &e) {
		return {req.id(), status::code_t::internal};
	}

	return resp;
}

void server::run(const std::string_view &ip, int port) {
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	if (auto r = uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0); r != 0) {
		throw std::runtime_error(std::string("Failed to bind to tcp address: ") + uv_strerror(r));
	}

	if (auto r = uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb); r != 0) {
		throw std::runtime_error(
			std::string("Failed to listen for connections: ") + uv_strerror(r));
	}

	uv_run(&_loop, UV_RUN_DEFAULT);
}
} // namespace grpcxx
