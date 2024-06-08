#include "conn.h"

#ifdef BOOST_ASIO_STANDALONE
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#else
#include <asio/as_tuple.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#endif

namespace grpcxx {
namespace asio {
namespace detail {
conn::conn(ASIO_NS::ip::tcp::socket &&sock) noexcept : _sock(std::move(sock)) {}

conn::requests_t conn::read(std::size_t n) {
	requests_t reqs;
	for (auto &ev : _session.read({_buf.data(), n})) {
		if (ev.stream_id <= 0) {
			continue;
		}

		if (ev.type == h2::detail::event::type_t::stream_close) {
			_streams.erase(ev.stream_id);
			continue;
		}

		streams_t::iterator it  = _streams.emplace(ev.stream_id, ev.stream_id).first;
		auto               &req = it->second;

		switch (ev.type) {
		case h2::detail::event::type_t::stream_data: {
			req.read(ev.data);
			break;
		}

		case h2::detail::event::type_t::stream_end: {
			reqs.push_front(std::move(req));
			_streams.erase(ev.stream_id);
			break;
		}

		case h2::detail::event::type_t::stream_header: {
			req.header(std::move(ev.header->name), std::move(ev.header->value));
			break;
		}

		default:
			break;
		}
	}

	return reqs;
}

ASIO_NS::awaitable<conn::requests_t> conn::reqs() noexcept {
	auto [ec, n] = co_await _sock.async_read_some(
		ASIO_NS::buffer(_buf.data(), _buf.capacity()), ASIO_NS::as_tuple(ASIO_NS::use_awaitable));

	if (ec) {
		// TODO: handle errors
		_eos = true;
		co_return requests_t{};
	}

	requests_t reqs;
	try {
		reqs = read(n);
	} catch (std::exception &) {
		// TODO: handle errors
		_eos = true;
		co_return requests_t{};
	}

	co_await write();
	co_return reqs;
}

ASIO_NS::awaitable<void> conn::write() {
	for (auto chunk = _session.pending(); chunk.size() > 0; chunk = _session.pending()) {
		co_await ASIO_NS::async_write(_sock, ASIO_NS::buffer(chunk), ASIO_NS::use_awaitable);
	}
}

ASIO_NS::awaitable<void> conn::write(::grpcxx::detail::response resp) noexcept {
	_session.headers(
		resp.id(),
		{
			{":status", "200"},
			{"content-type", "application/grpc"},
		});

	_session.data(resp.id(), resp.bytes());
	co_await write();

	const auto &status = resp.status();
	_session.trailers(
		resp.id(),
		{
			{"grpc-status", status},
			{"grpc-status-details-bin", status.details()},
		});

	co_await write();
}
} // namespace detail
} // namespace asio
} // namespace grpcxx
