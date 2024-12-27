#pragma once

#include "../server_base.h"

#include "loop.h"
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

	virtual ~server() noexcept;

	uv_loop_t *listen(std::string_view ip, int port);
	uv_loop_t *listen(uv_os_sock_t &&sock);

	void run(std::string_view ip, int port, std::stop_token token = {});
	void run(std::stop_token token = {});

	/// Run against an existing socket handle.
	///
	/// This should be useful when using socket-activation and the socket already exists at the
	/// application start (e.g. when using the systemd socket activation protocol), or when it is
	/// necessary to do additional setup on the socket, such as setting keep-alive options.
	///
	/// Please note that this will take ownership of the socket handle, and close it as necessary.
	///
	void run(uv_os_sock_t &&sock, std::stop_token token = {});

private:
	static constexpr int      TCP_LISTEN_BACKLOG         = 128;
	static constexpr uint64_t SHUTDOWN_CHECK_INTERVAL_MS = 100;

	static void conn_cb(uv_stream_t *stream, int status);

	detail::coroutine conn(uv_stream_t *stream);

	void bind(std::string_view ip, int port);
	void listen();
	void open(uv_os_sock_t &&sock);

	uv_tcp_t        _handle;
	uv_timer_t      _timer;
	std::stop_token _token;

	detail::loop      _loop;
	detail::scheduler _scheduler;
};
} // namespace uv
} // namespace grpcxx
