#include "stream.h"

#include <vector>

namespace grpcxx {
namespace h2 {
stream::stream(const id_t &id, nghttp2_session *session) noexcept : _id(id), _session(session) {}

void stream::send(const data_t &data) {
	buffer = data;

	nghttp2_data_provider provider;
	provider.source.ptr    = &buffer;
	provider.read_callback = read_cb;

	if (auto r = nghttp2_submit_data(_session, NGHTTP2_FLAG_NONE, id(), &provider); r != 0) {
		throw std::runtime_error(nghttp2_strerror(r));
	}

	if (auto r = nghttp2_session_send(_session); r != 0) {
		throw std::runtime_error(nghttp2_strerror(r));
	}
}

void stream::send(const headers_t &headers, bool eos) const {
	std::vector<nghttp2_nv> nv;
	nv.reserve(headers.size());

	for (const auto &[name, value] : headers) {
		nv.push_back({
			const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(name.data())),
			const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(value.data())),
			name.size(),
			value.size(),
			NGHTTP2_NV_FLAG_NONE,
		});
	}

	uint8_t flags = NGHTTP2_FLAG_NONE;
	if (eos) {
		flags |= NGHTTP2_FLAG_END_STREAM;
	}

	if (auto r =
			nghttp2_submit_headers(_session, flags, id(), nullptr, nv.data(), nv.size(), nullptr);
		r != 0) {
		throw std::runtime_error(nghttp2_strerror(r));
	}

	if (auto r = nghttp2_session_send(_session); r != 0) {
		throw std::runtime_error(nghttp2_strerror(r));
	}
}

ssize_t stream::read_cb(
	nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length, uint32_t *data_flags,
	nghttp2_data_source *source, void *) {
	auto *buffer = static_cast<data_t *>(source->ptr);

	if (length >= buffer->size()) {
		length = buffer->size();
		*data_flags |= NGHTTP2_DATA_FLAG_EOF;
	}

	std::memcpy(buf, buffer->data(), length);
	buffer->erase(0, length);

	return length;
}
} // namespace h2
} // namespace grpcxx
