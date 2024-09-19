#pragma once

#include "event.h"

#include <nghttp2/nghttp2.h>

#include <cstdint>
#include <string>
#include <vector>

namespace grpcxx {
namespace h2 {
namespace detail {
class session {
public:
	using events_t = std::vector<event>;

	session();
	session(const session &) = delete;

	~session();

	void headers(int32_t stream_id, detail::headers hdrs) const;
	void data(int32_t stream_id, std::string &&data);
	void trailers(int32_t stream_id, detail::headers hdrs) const;

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

	static ssize_t read_cb(
		nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length,
		uint32_t *data_flags, nghttp2_data_source *source, void *);

	static int stream_close_cb(
		nghttp2_session *session, int32_t stream_id, uint32_t error_code, void *vsess);

	void emit(event &&ev) noexcept;

	std::string      _data;
	events_t         _events;
	nghttp2_session *_session;
};
} // namespace detail
} // namespace h2
} // namespace grpcxx
