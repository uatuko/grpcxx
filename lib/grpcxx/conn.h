#pragma once

#include <memory>

#include <uv.h>

#include "h2/session.h"

#include "writer.h"

namespace grpcxx {
namespace detail {
class conn {
public:
	conn(uv_stream_t *stream);
	conn(const conn &) = delete;

	h2::session &session() noexcept { return _session; }

	writer write(std::string_view data) {
		return {std::reinterpret_pointer_cast<uv_stream_t>(_handle), data};
	}

private:
	template <std::size_t N> class buffer_t {
	public:
		constexpr char       *data() noexcept { return &_data[0]; }
		constexpr std::size_t capacity() const noexcept { return N; }

	private:
		char _data[N];
	};

	struct deleter {
		void operator()(void *handle) const noexcept {
			uv_close(static_cast<uv_handle_t *>(handle), [](uv_handle_t *handle) {
				delete reinterpret_cast<uv_tcp_t *>(handle);
			});
		}
	};

	static void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
	static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);

	buffer_t<1024>            _buf; // FIXME: make size configurable
	std::shared_ptr<uv_tcp_t> _handle;
	h2::session               _session;
};
} // namespace detail
} // namespace grpcxx
