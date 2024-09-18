#pragma once

#include <nghttp2/nghttp2.h>

#include <uv.h>

#include <cstdint>
#include <map>
#include <string>

struct h2_stream_t {
	using headers_t = std::map<std::string, std::string>;

	headers_t   headers;
	std::string data;
};

struct h2_context_t {
	nghttp2_session *session;
	uv_stream_t     *uv_stream;

	std::map<int32_t, h2_stream_t> streams;
};

// libuv callbacks
void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void on_close(uv_handle_t *handle);
void on_conn(uv_stream_t *stream, int status);
void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
void on_write(uv_write_t *req, int status);

// nghttp2 callbacks
ssize_t on_data_read(
	nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length, uint32_t *data_flags,
	nghttp2_data_source *source, void *vctx);

int on_session_data_chunk(
	nghttp2_session *session, uint8_t flags, int32_t stream_id, const uint8_t *data, size_t len,
	void *vctx);

int on_session_frame(nghttp2_session *session, const nghttp2_frame *frame, void *vctx);

int on_session_header(
	nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
	const uint8_t *value, size_t valuelen, uint8_t flags, void *vctx);

ssize_t on_session_send(
	nghttp2_session *session, const uint8_t *data, size_t length, int flags, void *vctx);

int on_session_stream_close(
	nghttp2_session *session, int32_t stream_id, uint32_t error_code, void *vctx);
