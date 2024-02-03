#include "conn.h"

namespace grpcxx {
namespace uv {
namespace detail {
conn::conn(uv_stream_t *stream) : _handle(new uv_tcp_t{}, deleter{}) {
	_buffer.reserve(1024); // FIXME: make size configurable
	uv_tcp_init(stream->loop, _handle.get());

	if (auto r = uv_accept(stream, reinterpret_cast<uv_stream_t *>(_handle.get())); r != 0) {
		throw std::runtime_error(std::string("Failed to accept connection: ") + uv_strerror(r));
	}
}

void conn::buffer() noexcept {
	for (auto chunk = _session.pending(); chunk.size() > 0; chunk = _session.pending()) {
		_buffer.append(chunk);
	}
}

task conn::flush() {
	if (_buffer.empty()) {
		co_return;
	}

	co_await write(_buffer);
	_buffer.clear();
}

conn::requests_t conn::read(std::string_view bytes) {
	requests_t reqs;
	for (auto &ev : _session.read(bytes)) {
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

	buffer();

	return reqs;
}

reader conn::reader() const noexcept {
	return {std::reinterpret_pointer_cast<uv_stream_t>(_handle)};
}

void conn::write(::grpcxx::detail::response resp) noexcept {
	_session.headers(
		resp.id(),
		{
			{":status", "200"},
			{"content-type", "application/grpc"},
		});

	_session.data(resp.id(), resp.bytes());
	buffer();

	const auto &status = resp.status();
	_session.trailers(
		resp.id(),
		{
			{"grpc-status", status},
			{"grpc-status-details-bin", status.details()},
		});

	buffer();
}

writer conn::write(std::string_view bytes) const noexcept {
	return {std::reinterpret_pointer_cast<uv_stream_t>(_handle), bytes};
}
} // namespace detail
} // namespace uv
} // namespace grpcxx
