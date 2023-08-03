#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <uv.h>

#include "service.h"
#include "status.h"

namespace grpcxx {
// Forward declarations
namespace h2 {
class conn;
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

	void run(const std::string_view &ip, int port);

private:
	static void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
	static void close_cb(uv_handle_t *handle);
	static void conn_cb(uv_stream_t *server, int status);
	static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
	static void write_cb(uv_write_t *req, int status);

	void   event(const h2::event &ev);
	size_t write(uv_stream_t *handle, const uint8_t *data, size_t size);

	void send(std::shared_ptr<h2::stream> stream, const status &s, const data_t &msg = {});

	uv_tcp_t  _handle;
	uv_loop_t _loop;

	services_t _services;
};
} // namespace grpcxx
