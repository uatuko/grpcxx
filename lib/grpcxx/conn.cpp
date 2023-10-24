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

reader conn::read() const noexcept {
	return {std::reinterpret_pointer_cast<uv_stream_t>(_handle)};
}

writer conn::write(std::string_view bytes) const noexcept {
	return {std::reinterpret_pointer_cast<uv_stream_t>(_handle), bytes};
}
} // namespace detail
} // namespace grpcxx
