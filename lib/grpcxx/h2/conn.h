#pragma once

#include <functional>

#include <nghttp2/nghttp2.h>

#include "event.h"
#include "stream.h"

namespace grpcxx {
namespace h2 {
class conn {
public:
	using streams_t = std::map<stream::id_t, std::shared_ptr<stream>>;

	using listener_t = std::function<void(const event &)>;
	using writer_t   = std::function<size_t(const uint8_t *, size_t)>;

	conn(listener_t &&listener, writer_t &&writer);
	~conn();

	void recv(const uint8_t *in, size_t size);
	void send();

private:
	static int data_recv_cb(
		nghttp2_session *session, uint8_t flags, int32_t stream_id, const uint8_t *data, size_t len,
		void *vconn);

	static int frame_recv_cb(nghttp2_session *session, const nghttp2_frame *frame, void *vconn);

	static int header_cb(
		nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
		const uint8_t *value, size_t valuelen, uint8_t flags, void *vconn);

	static ssize_t send_cb(
		nghttp2_session *session, const uint8_t *data, size_t length, int flags, void *vconn);

	inline void emit(const event &ev) { _listener(ev); }

	nghttp2_session *_session;
	streams_t        _streams;

	listener_t _listener;
	writer_t   _writer;
};
} // namespace h2
} // namespace grpcxx
