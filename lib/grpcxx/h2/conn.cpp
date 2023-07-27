#include "conn.h"

#include <array>

namespace grpcxx {
namespace h2 {
conn::conn(listener_t &&listener, writer_t &&writer) :
	_listener(std::move(listener)), _writer(std::move(writer)) {
	// Initialise HTTP/2 session
	nghttp2_session_callbacks *callbacks;
	nghttp2_session_callbacks_new(&callbacks);

	nghttp2_session_callbacks_set_on_header_callback(callbacks, header_cb);
	nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, data_recv_cb);
	nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, frame_recv_cb);
	nghttp2_session_callbacks_set_send_callback(callbacks, send_cb);

	nghttp2_session_server_new(&_session, callbacks, this);

	nghttp2_session_callbacks_del(callbacks);

	// Send HTTP/2 client connection header
	std::array<nghttp2_settings_entry, 1> iv = {
		// FIXME: make concurrent streams configurable (and default to more than 1?)
		{NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 1},
	};

	auto r = nghttp2_submit_settings(_session, NGHTTP2_FLAG_NONE, iv.data(), iv.size());
	if (r != 0) {
		throw std::runtime_error(nghttp2_strerror(r));
	}
}

conn::~conn() {
	nghttp2_session_del(_session);
}

int conn::data_recv_cb(
	nghttp2_session *session, uint8_t flags, int32_t stream_id, const uint8_t *data, size_t len,
	void *vconn) {
	auto *conn   = reinterpret_cast<class conn *>(vconn);
	auto &stream = conn->_streams.at(stream_id);

	stream->data.insert(stream->data.end(), data, data + len);
	return 0;
}

int conn::frame_recv_cb(nghttp2_session *session, const nghttp2_frame *frame, void *vconn) {
	auto *conn = reinterpret_cast<class conn *>(vconn);
	if (!conn->_streams.contains(frame->hd.stream_id)) {
		return 0;
	}

	switch (frame->hd.type) {
	case NGHTTP2_DATA: {
		conn->emit({
			.stream = conn->_streams.at(frame->hd.stream_id),
			.type   = event_type::data,
		});

		break;
	}

	case NGHTTP2_HEADERS: {
		conn->emit({
			.stream = conn->_streams.at(frame->hd.stream_id),
			.type   = event_type::headers,
		});

		break;
	}

	default:
		break;
	}

	if (0 != (frame->hd.flags & NGHTTP2_FLAG_END_STREAM)) {
		conn->emit({
			.stream = conn->_streams.at(frame->hd.stream_id),
			.type   = event_type::eos,
		});
	}

	return 0;
}

int conn::header_cb(
	nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
	const uint8_t *value, size_t valuelen, uint8_t flags, void *vconn) {
	auto *conn = reinterpret_cast<class conn *>(vconn);
	if (!conn->_streams.contains(frame->hd.stream_id)) {
		conn->_streams.insert({frame->hd.stream_id, std::make_shared<stream>(frame->hd.stream_id)});
	}

	auto &stream = conn->_streams.at(frame->hd.stream_id);

	stream->headers.insert({
		{reinterpret_cast<const char *>(name), namelen},
		{reinterpret_cast<const char *>(value), valuelen},
	});

	return 0;
}

void conn::recv(const uint8_t *in, size_t size) {
	auto n = nghttp2_session_mem_recv(_session, in, size);
	if (n < 0) {
		throw std::runtime_error(nghttp2_strerror(static_cast<int>(n)));
	}
}

void conn::send() {
	auto r = nghttp2_session_send(_session);
	if (r != 0) {
		throw std::runtime_error(nghttp2_strerror(r));
	}
}

ssize_t conn::send_cb(
	nghttp2_session *session, const uint8_t *data, size_t length, int flags, void *vconn) {
	auto *conn = reinterpret_cast<class conn *>(vconn);
	return conn->_writer(data, length);
}
} // namespace h2
} // namespace grpcxx
