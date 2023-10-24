#pragma once

#include <string_view>

#include <nghttp2/nghttp2.h>

namespace grpcxx {
namespace h2 {
class buffer {
public:
	buffer() = default;
	buffer(std::string_view data);

	static ssize_t read_cb(
		nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length,
		uint32_t *data_flags, nghttp2_data_source *source, void *);

	const auto *data() const noexcept { return _data.data() + _head; }
	size_t      size() const noexcept { return _data.size() - _head; }

	size_t consume(size_t n) noexcept {
		if (n > size()) {
			n = size();
		}

		_head += n;
		return n;
	}

private:
	std::string_view _data;
	size_t           _head;
};
} // namespace h2
} // namespace grpcxx
