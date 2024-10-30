#pragma once

#include "../server_base.h"

#include "scheduler.h"

#include <uv.h>

#include <string_view>
#include <thread>

namespace grpcxx {
namespace uv {
// Forward declarations
namespace detail {
struct coroutine;
}

class server : public ::grpcxx::server_base {
public:
	server(const server &) = delete;
	server(std::size_t n = std::thread::hardware_concurrency()) noexcept;

	void run(uv_tcp_t &&handle);
	void run(std::string_view ip, int port);

private:
	struct loop_t {
		loop_t() { uv_loop_init(&_loop); }

		operator uv_loop_t *() noexcept { return &_loop; }

		uv_loop_t _loop;
	};

	static void conn_cb(uv_stream_t *stream, int status);

	detail::coroutine conn(uv_stream_t *stream);

	uv_tcp_t _handle;
	loop_t   _loop;

	detail::scheduler _scheduler;
};
} // namespace uv
} // namespace grpcxx
