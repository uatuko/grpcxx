#pragma once

#include "../server_base.h"

#include "scheduler.h"

#include <uv.h>

#include <stop_token>
#include <string_view>

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

	/// Allow running against an existing uv_tcp_t handle
	///
	/// This is especially useful when using socket-activation
	/// (when the socket already exists at the application start,
	/// e.g. when using the systemd socket activation protocol),
	/// or when it is necessary to do additional setup on the socket,
	/// such as setting keep-alive options.
	///
	/// Please note that this will take ownership of the file
	/// descriptor, and close it as necessary.
	///
	void run(int fd, std::stop_token stop_token = {});

	void run(std::string_view ip, int port, std::stop_token stop_token = {});

private:
	static constexpr int      TCP_LISTEN_BACKLOG         = 128;
	static constexpr uint64_t SHUTDOWN_CHECK_INTERVAL_MS = 100;

	struct loop_t {
		loop_t();
		~loop_t() noexcept;

		operator uv_loop_t *() noexcept { return &_loop; }

		uv_loop_t _loop;
	};

	static void conn_cb(uv_stream_t *stream, int status);

	void run(uv_tcp_t &handle, std::stop_token stop_token = {});

	detail::coroutine conn(uv_stream_t *stream);

	uv_tcp_t   _handle;
	loop_t     _loop;
	uv_timer_t _check_stop_timer;

	detail::scheduler _scheduler;
};
} // namespace uv
} // namespace grpcxx
