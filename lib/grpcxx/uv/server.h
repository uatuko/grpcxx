#pragma once

#include <string_view>
#include <thread>

#include <uv.h>

#include "../server_base.h"
#include "scheduler.h"

namespace grpcxx {
namespace uv {
namespace detail {

// Forward declarations
struct coroutine;

class server : public ::grpcxx::detail::server_base {
public:
	server(const server &) = delete;
	server(std::size_t n = std::thread::hardware_concurrency()) noexcept;

	void run(std::string_view ip, int port);

private:
	struct loop_t {
		loop_t() { uv_loop_init(&_loop); }

		operator uv_loop_t *() noexcept { return &_loop; }

		uv_loop_t _loop;
	};

	static void conn_cb(uv_stream_t *stream, int status);

	coroutine conn(uv_stream_t *stream);

	uv_tcp_t _handle;
	loop_t   _loop;

	scheduler _scheduler;
};
} // namespace detail
} // namespace uv
} // namespace grpcxx
