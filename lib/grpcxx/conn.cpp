#include "conn.h"

#include <string>

namespace grpcxx {
namespace detail {
conn::conn(uv_stream_t *stream) : _handle(new uv_tcp_t{}, deleter{}) {
	uv_tcp_init(stream->loop, _handle.get());

	if (auto r = uv_accept(stream, reinterpret_cast<uv_stream_t *>(_handle.get())); r != 0) {
		throw std::runtime_error(std::string("Failed to accept connection: ") + uv_strerror(r));
	}
}

conn::requests_t conn::read(std::string_view bytes) {
	requests_t reqs;
	for (auto &ev : _session.read(bytes)) {
		if (ev.stream_id <= 0) {
			continue;
		}

		if (ev.type == h2::event::type_t::stream_close) {
			_streams.erase(ev.stream_id);
			continue;
		}

		streams_t::iterator it  = _streams.emplace(ev.stream_id, ev.stream_id).first;
		auto               &req = it->second;

		switch (ev.type) {
		case h2::event::type_t::stream_data: {
			req.read(ev.data);
			break;
		}

		case h2::event::type_t::stream_end: {
			reqs.push_front(std::move(req));
			_streams.erase(ev.stream_id);
			break;
		}

		case h2::event::type_t::stream_header: {
			req.header(std::move(ev.header->name), std::move(ev.header->value));
			break;
		}

		default:
			break;
		}
	}

	return reqs;
}

reader conn::reader() const noexcept {
	return {std::reinterpret_pointer_cast<uv_stream_t>(_handle)};
}

writer conn::write(std::string_view bytes) const noexcept {
	return {std::reinterpret_pointer_cast<uv_stream_t>(_handle), bytes};
}
} // namespace detail
} // namespace grpcxx
