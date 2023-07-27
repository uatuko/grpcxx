#include "echo-server.h"

#include <cstdio>

uv_loop_t *loop;

void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
	buf->base = new char[suggested_size]();
	buf->len  = suggested_size;
}

void on_close(uv_handle_t *handle) {
	std::printf("on_close()\n");

	auto *ctx = reinterpret_cast<h2_context_t *>(handle->data);
	delete ctx;

	delete handle;
}

void on_conn(uv_stream_t *stream, int status) {
	std::printf("on_conn()\n");

	if (status < 0) {
		std::fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
		return;
	}

	auto *sock = new uv_tcp_t();
	uv_tcp_init(loop, sock);

	if (uv_accept(stream, reinterpret_cast<uv_stream_t *>(sock)) == 0) {
		// Initialise HTTP/2 session
		nghttp2_session_callbacks *callbacks;
		nghttp2_session_callbacks_new(&callbacks);

		nghttp2_session_callbacks_set_on_header_callback(callbacks, on_session_header);
		nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, on_session_frame);
		nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, on_session_data_chunk);
		nghttp2_session_callbacks_set_send_callback(callbacks, on_session_send);
		nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, on_session_stream_close);

		auto *ctx      = new h2_context_t();
		ctx->uv_stream = reinterpret_cast<uv_stream_t *>(sock);
		sock->data     = ctx;
		nghttp2_session_server_new(&ctx->session, callbacks, ctx);

		nghttp2_session_callbacks_del(callbacks);

		// Send HTTP/2 client connection header
		std::array<nghttp2_settings_entry, 1> iv = {
			{NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, 1},
		};

		auto rv = nghttp2_submit_settings(ctx->session, NGHTTP2_FLAG_NONE, iv.data(), iv.size());
		if (rv != 0) {
			std::fprintf(stderr, "Fatal error: %s\n", nghttp2_strerror(rv));
			uv_close((uv_handle_t *)sock, on_close);
			return;
		}

		uv_read_start(reinterpret_cast<uv_stream_t *>(sock), on_alloc, on_read);
	} else {
		uv_close((uv_handle_t *)sock, on_close);
	}
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
	if (nread < 0) {
		if (nread != UV_EOF) {
			std::fprintf(stderr, "Read error: %s\n", uv_err_name(nread));
		}

		uv_close((uv_handle_t *)stream, on_close);
		delete[] buf->base;
		return;
	}

	auto *ctx = reinterpret_cast<h2_context_t *>(stream->data);
	auto  n   = nghttp2_session_mem_recv(ctx->session, (const uint8_t *)buf->base, nread);
	delete[] buf->base;

	if (n < 0) {
		std::fprintf(stderr, "Fatal error: %s\n", nghttp2_strerror(static_cast<int>(n)));
		uv_close((uv_handle_t *)stream, on_close);
		return;
	}

	auto rv = nghttp2_session_send(ctx->session);
	if (rv != 0) {
		std::fprintf(stderr, "Fatal error: %s\n", nghttp2_strerror(rv));
		uv_close((uv_handle_t *)stream, on_close);
		return;
	}
}

void on_write(uv_write_t *req, int status) {
	if (status) {
		std::fprintf(stderr, "Write error: %s\n", uv_strerror(status));
	}

	auto *buf = reinterpret_cast<char *>(req->data);
	delete[] buf;

	delete req;
}

ssize_t on_data_read(
	nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length, uint32_t *data_flags,
	nghttp2_data_source *source, void *vctx) {
	auto       *ctx    = reinterpret_cast<h2_context_t *>(vctx);
	const auto &stream = ctx->streams[stream_id];

	if (stream.data.size() > length) {
		std::fprintf(
			stderr,
			"Warning: not enough capacity to send all the data (want: %zu, have: %zu)\n",
			stream.data.size(),
			length);
		*data_flags |= NGHTTP2_DATA_FLAG_EOF;
		return 0;
	}

	std::memcpy(buf, stream.data.data(), length);
	*data_flags |= NGHTTP2_DATA_FLAG_EOF;
	return stream.data.size();
}

int on_session_data_chunk(
	nghttp2_session *session, uint8_t flags, int32_t stream_id, const uint8_t *data, size_t len,
	void *vctx) {
	auto *ctx    = reinterpret_cast<h2_context_t *>(vctx);
	auto &stream = ctx->streams[stream_id];

	stream.data.append(reinterpret_cast<const char *>(data), len);
	return 0;
}

int on_session_frame(nghttp2_session *session, const nghttp2_frame *frame, void *vctx) {
	auto *ctx = reinterpret_cast<h2_context_t *>(vctx);
	switch (frame->hd.type) {
	case NGHTTP2_DATA: {
		std::printf("[frame](%d) data\n", frame->hd.stream_id);

		auto &stream = ctx->streams[frame->hd.stream_id];
		std::printf("  size: %ld\n", stream.data.size());
		std::printf("  [%s]\n", stream.data.c_str());

		// Send response (and end stream)
		{
			std::array<nghttp2_nv, 2> headers = {{
				{
					(uint8_t *)":status",
					(uint8_t *)"200",
					sizeof(":status") - 1,
					sizeof("200") - 1,
					NGHTTP2_NV_FLAG_NONE,
				},
				{
					(uint8_t *)"content-type",
					reinterpret_cast<uint8_t *>(stream.headers["content-type"].data()),
					sizeof("content-type") - 1,
					stream.headers["content-type"].size(),
					NGHTTP2_NV_FLAG_NONE,
				},
			}};

			nghttp2_data_provider data;
			data.read_callback = on_data_read;

			auto rv = nghttp2_submit_response(
				session, frame->hd.stream_id, headers.data(), headers.size(), &data);
			if (rv != 0) {
				std::fprintf(
					stderr, "Failed to submit response, error: %s\n", nghttp2_strerror(rv));
			}
		}

		break;
	}

	case NGHTTP2_HEADERS: {
		std::printf("[frame](%d) headers\n", frame->hd.stream_id);

		const auto &stream = ctx->streams[frame->hd.stream_id];
		for (const auto &[name, value] : stream.headers) {
			std::printf("  %s: %s\n", name.c_str(), value.c_str());
		}

		break;
	}

	default:
		break;
	}

	return 0;
}

int on_session_header(
	nghttp2_session *session, const nghttp2_frame *frame, const uint8_t *name, size_t namelen,
	const uint8_t *value, size_t valuelen, uint8_t flags, void *vctx) {
	auto *ctx    = reinterpret_cast<h2_context_t *>(vctx);
	auto &stream = ctx->streams[frame->hd.stream_id];

	stream.headers.insert({
		{reinterpret_cast<const char *>(name), namelen},
		{reinterpret_cast<const char *>(value), valuelen},
	});

	return 0;
}

ssize_t on_session_send(
	nghttp2_session *session, const uint8_t *data, size_t length, int flags, void *vctx) {
	auto *ctx = reinterpret_cast<h2_context_t *>(vctx);
	auto *req = new uv_write_t();

	auto *buf = new char[length]();
	std::memcpy(buf, data, length);

	req->data = reinterpret_cast<void *>(buf);

	auto uv_buf = uv_buf_init(buf, length);
	uv_write(reinterpret_cast<uv_write_t *>(req), ctx->uv_stream, &uv_buf, 1, on_write);

	return length;
}

int on_session_stream_close(
	nghttp2_session *session, int32_t stream_id, uint32_t error_code, void *vctx) {
	std::printf("on_session_stream_close(), stream_id: %d\n", stream_id);

	return 0;
}

int main() {
	loop = uv_default_loop();

	uv_tcp_t sock;
	uv_tcp_init(loop, &sock);

	struct sockaddr_in addr;
	uv_ip4_addr("127.0.0.1", 7000, &addr);

	uv_tcp_bind(&sock, reinterpret_cast<const sockaddr *>(&addr), 0);

	auto r = uv_listen(reinterpret_cast<uv_stream_t *>(&sock), 128, on_conn);
	if (r) {
		std::fprintf(stderr, "Listen error: %s\n", uv_strerror(r));
		return 1;
	}

	return uv_run(loop, UV_RUN_DEFAULT);
}
