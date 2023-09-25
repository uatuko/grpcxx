#pragma once

#include <memory>

#include <uv.h>

#include "reader.h"
#include "writer.h"

namespace grpcxx {
namespace detail {
class conn {
public:
	conn(uv_stream_t *stream);
	conn(const conn &) = delete;

	operator bool() const noexcept { return _reader; }

	reader &read() noexcept { return _reader; }

	writer write(std::string_view data) {
		return {std::reinterpret_pointer_cast<uv_stream_t>(_handle), data};
	}

private:
	struct deleter {
		void operator()(void *handle) const noexcept {
			uv_close(static_cast<uv_handle_t *>(handle), [](uv_handle_t *handle) {
				delete reinterpret_cast<uv_tcp_t *>(handle);
			});
		}
	};

	static void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
	static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);

	std::shared_ptr<uv_tcp_t> _handle;
	reader                    _reader;
};
} // namespace detail
} // namespace grpcxx
