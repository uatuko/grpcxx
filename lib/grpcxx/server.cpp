#include "server.h"

#include "conn.h"
#include "coroutine.h"
#include "request.h"
#include "response.h"

namespace grpcxx {
server::server(std::size_t n) noexcept : _scheduler(_loop, n) {
	uv_tcp_init(_loop, &_handle);
	_handle.data = this;
}

detail::coroutine server::conn(uv_stream_t *stream) {
	std::string buf;
	buf.reserve(1024);

	detail::conn c(stream);
	auto         reader  = c.reader();
	auto        &session = c.session();
	while (reader) {
		auto bytes = co_await reader;
		if (bytes.empty()) {
			continue;
		}

		co_await _scheduler.run([&] {
			for (auto chunk = session.pending(); chunk.size() > 0; chunk = session.pending()) {
				buf.append(chunk);
			}

			for (auto &req : c.read(bytes)) {
				auto resp = process(req);
				session.headers(
					resp.id(),
					{
						{":status", "200"},
						{"content-type", "application/grpc"},
					});

				session.data(resp.id(), resp.bytes());
				for (auto chunk = session.pending(); chunk.size() > 0; chunk = session.pending()) {
					buf.append(chunk);
				}

				const auto &status = resp.status();
				session.trailers(
					resp.id(),
					{
						{"grpc-status", status},
						{"grpc-status-details-bin", status.details()},
					});

				for (auto chunk = session.pending(); chunk.size() > 0; chunk = session.pending()) {
					buf.append(chunk);
				}
			}
		});

		if (!buf.empty()) {
			co_await c.write(buf);
			buf.clear();
		}
	}
}

void server::conn_cb(uv_stream_t *stream, int status) {
	if (status < 0) {
		return;
	}

	auto *s = static_cast<server *>(stream->data);
	s->conn(stream);
}

detail::response server::process(const detail::request &req) const noexcept {
	if (!req) {
		return {req.id(), status::code_t::invalid_argument};
	}

	auto it = _services.find(req.service());
	if (it == _services.end()) {
		return {req.id(), status::code_t::not_found};
	}

	context          ctx(req);
	detail::response resp(req.id());
	try {
		auto r = it->second(ctx, req.method(), req.data());
		resp.status(std::move(r.first));
		resp.data(std::move(r.second));
	} catch (std::exception &e) {
		return {req.id(), status::code_t::internal};
	}

	return resp;
}

void server::run(const std::string_view &ip, int port) {
	struct sockaddr_in addr;
	uv_ip4_addr(ip.data(), port, &addr);

	if (auto r = uv_tcp_bind(&_handle, reinterpret_cast<const sockaddr *>(&addr), 0); r != 0) {
		throw std::runtime_error(std::string("Failed to bind to tcp address: ") + uv_strerror(r));
	}

	if (auto r = uv_listen(reinterpret_cast<uv_stream_t *>(&_handle), 128, conn_cb); r != 0) {
		throw std::runtime_error(
			std::string("Failed to listen for connections: ") + uv_strerror(r));
	}

	uv_run(_loop, UV_RUN_DEFAULT);
}
} // namespace grpcxx
