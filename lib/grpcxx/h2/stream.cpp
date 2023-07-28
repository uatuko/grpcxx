#include "stream.h"

namespace grpcxx {
namespace h2 {
stream::stream(const id_t &id, nghttp2_session *session) noexcept : _id(id), _session(session) {}

void stream::send(const headers_t &headers, const data_t &data) const {
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

	auto r = nghttp2_submit_response(_session, id(), nv.data(), nv.size(), nullptr);
	if (r != 0) {
		throw std::runtime_error(nghttp2_strerror(r));
	}
}
} // namespace h2
} // namespace grpcxx
