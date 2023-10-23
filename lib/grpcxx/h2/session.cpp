#include "session.h"

#include <array>

namespace grpcxx {
namespace h2 {
session::session() {
	// Initialise HTTP/2 session
	nghttp2_session_callbacks *callbacks;
	nghttp2_session_callbacks_new(&callbacks);

	nghttp2_session_callbacks_set_on_header_callback(callbacks, header_cb);
	nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, data_recv_cb);
	nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, frame_recv_cb);
	nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, stream_close_cb);
	nghttp2_session_callbacks_set_send_callback(callbacks, send_cb);

	nghttp2_session_server_new(&_session, callbacks, this);

	nghttp2_session_callbacks_del(callbacks);

	// Send HTTP/2 client connection header
	std::array<nghttp2_settings_entry, 1> iv = {
		// FIXME: make concurrent streams configurable
		{NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 10},
	};

	auto r = nghttp2_submit_settings(_session, NGHTTP2_FLAG_NONE, iv.data(), iv.size());
	if (r != 0) {
		throw std::runtime_error(nghttp2_strerror(r));
	}
}

session::~session() {
	nghttp2_session_del(_session);
}

bool session::await_ready() const noexcept {
	return !_events.empty();
}

event session::await_resume() noexcept {
	_h = nullptr;

	if (_events.empty()) {
		return {};
	}

	auto ev = _events.front();
	_events.pop();

	return ev;
}

void session::await_suspend(std::coroutine_handle<> h) noexcept {
	_h = h;
}

int session::data_recv_cb(
	nghttp2_session *session, uint8_t flags, int32_t stream_id, const uint8_t *data, size_t len,
	void *vsess) {
	auto *sess = static_cast<class session *>(vsess);
	sess->emit({
		.data      = {reinterpret_cast<const char *>(data), len},
		.stream_id = stream_id,
		.type      = event::type_t::stream_data,
	});

	return 0;
}

void session::emit(event &&ev) noexcept {
	_events.push(ev);
	resume();
}

void session::end() noexcept {
	_eos = true;
	emit({
		.type = event::type_t::session_end,
	});
}

void session::error(int code) noexcept {
	_error = true;
	emit({
		.error = nghttp2_strerror(code),
		.type  = event::type_t::session_error,
	});
}

int session::frame_recv_cb(nghttp2_session *session, const nghttp2_frame *frame, void *vsess) {
	auto *sess = static_cast<class session *>(vsess);
	if (0 != (frame->hd.flags & NGHTTP2_FLAG_END_STREAM)) {
		sess->emit({
			.stream_id = {frame->hd.stream_id},
			.type      = event::type_t::stream_end,
		});
	}

	return 0;
}

int session::header_cb(
	nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
	const uint8_t *value, size_t valuelen, uint8_t flags, void *vsess) {
	auto *sess = static_cast<class session *>(vsess);
	sess->emit({
		.header =
			header{
				.name  = {reinterpret_cast<const char *>(name), namelen},
				.value = {reinterpret_cast<const char *>(value), valuelen},
			},
		.stream_id = {frame->hd.stream_id},
		.type      = event::type_t::stream_header,
	});

	return 0;
}

void session::recv(const uint8_t *in, size_t size) noexcept {
	if (auto n = nghttp2_session_mem_recv(_session, in, size); n < 0) {
		return error(n);
	}

	if (auto r = nghttp2_session_send(_session); r != 0) {
		return error(r);
	}
}

void session::resume() const noexcept {
	if (_h) {
		_h();
	}
}

ssize_t session::send_cb(
	nghttp2_session *session, const uint8_t *data, size_t length, int flags, void *vsess) {
	std::printf("session::send_cb(), length: %zu\n", length);
	auto *sess = static_cast<class session *>(vsess);
	sess->emit({
		.data = {reinterpret_cast<const char *>(data), length},
		.type = event::type_t::session_write,
	});

	return length;
}

int session::stream_close_cb(
	nghttp2_session *session, int32_t stream_id, uint32_t error_code, void *vsess) {
	std::printf("session::stream_close_db()\n");
	return 0;
}
} // namespace h2
} // namespace grpcxx
