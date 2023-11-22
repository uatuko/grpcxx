#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <uv.h>

#include "pool.h"
#include "service.h"

namespace grpcxx {
// Forward declarations
namespace detail {
struct coroutine;

class request;
class response;
} // namespace detail

class server {
public:
	using fn_t = std::function<std::pair<status, std::string>(std::string_view, std::string_view)>;
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
	static void conn_cb(uv_stream_t *stream, int status);

	detail::coroutine accept(uv_stream_t *stream);
	detail::response  process(const detail::request &req) const noexcept;

	uv_tcp_t  _handle;
	uv_loop_t _loop;

	detail::pool _pool;
	services_t   _services;
};
} // namespace grpcxx
