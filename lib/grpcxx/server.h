#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include <uv.h>

#include "service.h"

namespace grpcxx {
// Forward declarations
namespace detail {
struct task;
} // namespace detail

namespace h2 {
struct event;
class stream;
} // namespace h2

class server {
public:
	using data_t     = std::string;
	using fn_t       = std::function<std::pair<status, data_t>(std::string_view, const data_t &)>;
	using services_t = std::unordered_map<std::string_view, fn_t>;

	server();

	template <typename S> void add(S &s) {
		fn_t fn = std::bind(&S::call, &s, std::placeholders::_1, std::placeholders::_2);
		_services.insert({s.name(), fn});
	}

	const auto &services() const noexcept { return _services; }

	void run(const std::string_view &ip, int port);

private:
	static void conn_cb(uv_stream_t *stream, int status);

	detail::task conn(uv_stream_t *stream);

	uv_tcp_t  _handle;
	uv_loop_t _loop;

	services_t _services;
};
} // namespace grpcxx
