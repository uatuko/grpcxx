#pragma once

#include <coroutine>
#include <queue>

#include <nghttp2/nghttp2.h>

#include "event.h"

namespace grpcxx {
namespace h2 {
class session {
public:
	using events_t = std::queue<event>;

	session();
	session(const session &) = delete;

	~session();

	operator bool() const noexcept { return !(_eos || _error); }

	bool  await_ready() const noexcept;
	void  await_suspend(std::coroutine_handle<> h) noexcept;
	event await_resume() noexcept;

	void end() noexcept;
	void recv(const uint8_t *in, size_t size) noexcept;

private:
	static int data_recv_cb(
		nghttp2_session *session, uint8_t flags, int32_t stream_id, const uint8_t *data, size_t len,
		void *vsess);

	static int frame_recv_cb(nghttp2_session *session, const nghttp2_frame *frame, void *vsess);

	static int header_cb(
		nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
		const uint8_t *value, size_t valuelen, uint8_t flags, void *vsess);

	static ssize_t send_cb(
		nghttp2_session *session, const uint8_t *data, size_t length, int flags, void *vsess);

	static int stream_close_cb(
		nghttp2_session *session, int32_t stream_id, uint32_t error_code, void *vsess);

	void error(int code) noexcept;

	void emit(event &&ev) noexcept;
	void resume() const noexcept;

	bool _eos   = false;
	bool _error = false;

	events_t                _events;
	std::coroutine_handle<> _h;
	nghttp2_session        *_session;
};
} // namespace h2
} // namespace grpcxx
