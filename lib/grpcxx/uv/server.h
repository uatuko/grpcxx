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
	server(std::size_t n = std::thread::hardware_concurrency()) noexcept;

	server(const server &) = delete;

	virtual ~server() noexcept;

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
	void run(uv_os_sock_t sock, std::stop_token stop_token = {});

	void run(std::string_view ip, int port, std::stop_token stop_token = {});

protected:
	/// Lower-level API for integration in other event loops
	///
	/// If you use this constructor, you should also call `prepare()`
	/// instead of `run()` and start the loop yourself.
	///
	server(uv_loop_t &uv_loop, std::size_t n = std::thread::hardware_concurrency()) noexcept;

	/// Lower-level API for integration in other event loops
	///
	/// This API prepares the server for listening to connections,
	/// but does not run the internal loop.
	///
	/// This is useful for integration in other event loops
	/// (or nesting `uv_loop_t`s).
	///
	void prepare(uv_os_sock_t sock);

	/// Lower-level API for integration in other event loops
	///
	/// This API prepares the server for listening to connections
	/// over a pre-built, bound socket, but does not run the internal
	/// loop.
	///
	/// This is useful for integration in other event loops
	/// (or nesting `uv_loop_t`s).
	///
	void prepare(std::string_view ip, int port);

private:
	static constexpr int      TCP_LISTEN_BACKLOG         = 128;
	static constexpr uint64_t SHUTDOWN_CHECK_INTERVAL_MS = 100;

	static void conn_cb(uv_stream_t *stream, int status);

	void start_listening();

	void setup_stop_timer(std::stop_token stop_token) noexcept;

	detail::coroutine conn(uv_stream_t *stream);

	detail::loop_t  _loop;
	uv_tcp_t        _handle;
	uv_timer_t      _check_stop_timer;
	std::stop_token _stop_token;

	detail::scheduler _scheduler;
};
} // namespace uv
} // namespace grpcxx
