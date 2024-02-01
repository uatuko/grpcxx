#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include <uv.h>

#include "context.h"
#include "scheduler.h"
#include "status.h"

namespace grpcxx {
// Forward declarations
namespace detail {
struct coroutine;

class request;
class response;
} // namespace detail

class server {
public:
	using fn_t = std::function<std::pair<status, std::string>(
		context &, std::string_view, std::string_view)>;

	using services_t = std::unordered_map<std::string_view, fn_t>;

	server(const server &) = delete;
	server(std::size_t n = std::thread::hardware_concurrency()) noexcept;

	template <typename S> void add(S &s) {
		fn_t fn = std::bind_front(&S::call, &s);
		_services.insert({s.name(), fn});
	}

	const auto &services() const noexcept { return _services; }

	void run(const std::string_view &ip, int port);

private:
	struct loop_t {
		loop_t() { uv_loop_init(&_loop); }

		operator uv_loop_t *() noexcept { return &_loop; }
		uv_loop_t *operator&() noexcept { return &_loop; }

		uv_loop_t _loop;
	};

	static void conn_cb(uv_stream_t *stream, int status);

	detail::coroutine conn(uv_stream_t *stream);
	detail::response  process(const detail::request &req) const noexcept;

	uv_tcp_t _handle;
	loop_t   _loop;

	services_t _services;

	detail::scheduler _scheduler;
};
} // namespace grpcxx
