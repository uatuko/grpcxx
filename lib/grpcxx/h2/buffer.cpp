#include "buffer.h"

namespace grpcxx {
namespace h2 {
buffer::buffer(std::string_view data) : _data(data), _head(0) {}

ssize_t buffer::read_cb(
	nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length, uint32_t *data_flags,
	nghttp2_data_source *source, void *) {
	auto *b = static_cast<buffer *>(source->ptr);

	auto *data = b->data();
	auto  n    = b->consume(length);
	if (length > n) {
		*data_flags |= NGHTTP2_DATA_FLAG_EOF;
	}

	if (n > 0) {
		std::memcpy(buf, data, n);
	}

	return n;
}
} // namespace h2
} // namespace grpcxx
