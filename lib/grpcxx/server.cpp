#include "server.h"

#include <list>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "conn.h"
#include "coroutine.h"
#include "request.h"
#include "response.h"
#include "worker.h"

class threadpool {
public:
	explicit threadpool(const std::size_t threadCount) {
		for (std::size_t i = 0; i < threadCount; ++i) {
			std::thread worker_thread([this]() { this->loop(); });
			_threads.push_back(std::move(worker_thread));
		}
	}

	~threadpool() { shutdown(); }

	auto schedule() {
		struct awaiter {
			constexpr bool await_ready() const noexcept { return false; }
			constexpr void await_resume() const noexcept {}
			void           await_suspend(std::coroutine_handle<> coro) const noexcept {
                _pool->enqueue_task(coro);
			}

			threadpool *_pool;
		};

		return awaiter{this};
	}

private:
	void loop() {
		while (!_stop_thread) {
			std::unique_lock<std::mutex> lock(_mutex);

			while (!_stop_thread && _coros.size() == 0) {
				_cond.wait_for(lock, std::chrono::microseconds(100));
			}

			if (_stop_thread) {
				break;
			}

			auto coro = _coros.front();
			_coros.pop();

			lock.unlock();
			coro.resume();
		}
	}

	void enqueue_task(std::coroutine_handle<> coro) noexcept {
		std::unique_lock<std::mutex> lock(_mutex);
		_coros.emplace(coro);
		_cond.notify_one();
	}

	void shutdown() {
		_stop_thread = true;
		while (_threads.size() > 0) {
			std::thread &thread = _threads.back();
			if (thread.joinable()) {
				thread.join();
			}

			_threads.pop_back();
		}
	}

	std::list<std::thread> _threads;

	std::mutex                          _mutex;
	std::condition_variable             _cond;
	std::queue<std::coroutine_handle<>> _coros;

	bool _stop_thread = false;
};

threadpool pool{std::thread::hardware_concurrency()};

namespace grpcxx {
server::server() {
	// TODO: error handling
	uv_loop_init(&_loop);
	uv_tcp_init(&_loop, &_handle);

	_handle.data = this;
}

detail::coroutine server::accept(uv_stream_t *stream) {
	uv_loop_t loop;
	uv_tcp_t  handle;

	uv_loop_init(&loop);
	loop.data = this;

	uv_tcp_init(&loop, &handle);

	if (auto r = uv_accept(stream, reinterpret_cast<uv_stream_t *>(&handle)); r != 0) {
		throw std::runtime_error(std::string("Failed to accept connection: ") + uv_strerror(r));
	}

	co_await pool.schedule();

	detail::conn c;
	handle.data = &c;

	uv_read_start(
		reinterpret_cast<uv_stream_t *>(&handle),
		[](uv_handle_t *handle, size_t, uv_buf_t *buf) {
			auto *c = static_cast<detail::conn *>(handle->data);
			c->alloc(buf);
		},
		[](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
			if (nread <= 0) {
				return;
			}

			auto *c = static_cast<detail::conn *>(stream->data);
			c->read(nread);
			c->write(stream);

			auto *s = static_cast<server *>(stream->loop->data);
			for (const auto &req : c->reqs()) {
				auto resp = s->process(req);
				c->write(stream, resp);
			}
		});

	uv_run(&loop, UV_RUN_DEFAULT);
}

void server::conn_cb(uv_stream_t *stream, int status) {
	if (status < 0) {
		return;
	}

	auto *s = static_cast<server *>(stream->data);
	s->accept(stream);
}

detail::response server::process(const detail::request &req) const noexcept {
	if (!req) {
		return {req.id(), status::code_t::invalid_argument};
	}

	auto it = _services.find(req.service());
	if (it == _services.end()) {
		return {req.id(), status::code_t::not_found};
	}

	detail::response resp(req.id());
	try {
		auto r = it->second(req.method(), req.data());
		resp.status(std::move(r.first));
		resp.data(std::move(r.second));
	} catch (std::exception &e) {
		return {req.id(), status::code_t::internal};
	}

	return resp;
}

void server::run(const std::string_view &ip, int port) {
	// TODO: error handling
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0);
	uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb);

	uv_run(&_loop, UV_RUN_DEFAULT);
}
} // namespace grpcxx
