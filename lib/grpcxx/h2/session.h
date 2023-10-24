#pragma once

#include <string_view>
#include <vector>

#include <nghttp2/nghttp2.h>

#include "buffer.h"
#include "event.h"

namespace grpcxx {
namespace h2 {
class session {
public:
	using events_t = std::vector<event>;

	session();
	session(const session &) = delete;

	~session();

	void headers(int32_t stream_id, h2::headers hdrs) const;
	void data(int32_t stream_id, buffer &buf) const;
	void trailers(int32_t stream_id, h2::headers hdrs) const;

	events_t         read(std::string_view bytes);
	std::string_view pending();

private:
	static int data_recv_cb(
		nghttp2_session *session, uint8_t flags, int32_t stream_id, const uint8_t *data, size_t len,
		void *vsess);

	static int frame_recv_cb(nghttp2_session *session, const nghttp2_frame *frame, void *vsess);

	static int header_cb(
		nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
		const uint8_t *value, size_t valuelen, uint8_t flags, void *vsess);

	static int stream_close_cb(
		nghttp2_session *session, int32_t stream_id, uint32_t error_code, void *vsess);

	void emit(event &&ev) noexcept;

	events_t         _events;
	nghttp2_session *_session;
};
} // namespace h2
} // namespace grpcxx
