#include "session.h"

#include <array>
#include <cstring>
#include <stdexcept>

namespace grpcxx {
namespace h2 {
namespace detail {
session::session() : _data(), _events(), _session(nullptr) {
	// Initialise HTTP/2 session
	nghttp2_session_callbacks *callbacks;
	nghttp2_session_callbacks_new(&callbacks);

	nghttp2_session_callbacks_set_on_header_callback(callbacks, header_cb);
	nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, data_recv_cb);
	nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, frame_recv_cb);
	nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, stream_close_cb);

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

void session::data(int32_t stream_id, std::string &&data) {
	_data = std::move(data);
	nghttp2_data_provider provider{
		.source =
			{
				.ptr = &_data,
			},
		.read_callback = read_cb,
	};

	if (auto r = nghttp2_submit_data(_session, NGHTTP2_FLAG_NONE, stream_id, &provider); r != 0) {
		throw std::runtime_error(std::string("Failed to submit data: ") + nghttp2_strerror(r));
	}
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
	_events.push_back(ev);
}

int session::frame_recv_cb(nghttp2_session *session, const nghttp2_frame *frame, void *vsess) {
	auto *sess = static_cast<class session *>(vsess);
	if (0 != (frame->hd.flags & NGHTTP2_FLAG_END_STREAM)) {
		sess->emit({
			.stream_id = frame->hd.stream_id,
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
		.stream_id = frame->hd.stream_id,
		.type      = event::type_t::stream_header,
		.header =
			header{
				.name  = {reinterpret_cast<const char *>(name), namelen},
				.value = {reinterpret_cast<const char *>(value), valuelen},
			},
	});

	return 0;
}

void session::headers(int32_t stream_id, detail::headers hdrs) const {
	std::vector<nghttp2_nv> nv;
	nv.reserve(hdrs.size());

	for (const auto &h : hdrs) {
		if (h.value.empty()) {
			continue;
		}

		nv.push_back({
			const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(h.name.data())),
			const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(h.value.data())),
			h.name.size(),
			h.value.size(),
			NGHTTP2_NV_FLAG_NO_COPY_NAME | NGHTTP2_NV_FLAG_NO_COPY_VALUE,
		});
	}

	if (auto r = nghttp2_submit_headers(
			_session, NGHTTP2_FLAG_NONE, stream_id, nullptr, nv.data(), nv.size(), nullptr);
		r != 0) {
		throw std::runtime_error(std::string("Failed to submit headers: ") + nghttp2_strerror(r));
	}
}

std::string_view session::pending() {
	const uint8_t *bytes;
	auto           n = nghttp2_session_mem_send(_session, &bytes);
	if (n < 0) {
		throw std::runtime_error(
			std::string("Failed to retrieve pending session data: ") + nghttp2_strerror(n));
	}

	return {reinterpret_cast<const char *>(bytes), static_cast<size_t>(n)};
}

session::events_t session::read(std::string_view bytes) {
	if (auto n = nghttp2_session_mem_recv(
			_session, reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());
		n < 0) {
		throw std::runtime_error(
			std::string("Failed to read session data: ") + nghttp2_strerror(n));
	}

	auto events = _events;
	_events.clear();

	return events;
}

ssize_t session::read_cb(
	nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length, uint32_t *data_flags,
	nghttp2_data_source *source, void *) {
	auto *str = static_cast<std::string *>(source->ptr);

	if (length >= str->size()) {
		length       = str->size();
		*data_flags |= NGHTTP2_DATA_FLAG_EOF;
	}

	std::memcpy(buf, str->data(), length);
	str->erase(0, length);

	return length;
}

int session::stream_close_cb(
	nghttp2_session *session, int32_t stream_id, uint32_t error_code, void *vsess) {
	auto *sess = static_cast<class session *>(vsess);
	sess->emit({
		.stream_id = stream_id,
		.type      = event::type_t::stream_close,
	});

	return 0;
}

void session::trailers(int32_t stream_id, detail::headers hdrs) const {
	std::vector<nghttp2_nv> nv;
	nv.reserve(hdrs.size());

	for (const auto &h : hdrs) {
		if (h.value.empty()) {
			continue;
		}

		nv.push_back({
			const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(h.name.data())),
			const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(h.value.data())),
			h.name.size(),
			h.value.size(),
			NGHTTP2_NV_FLAG_NO_COPY_NAME | NGHTTP2_NV_FLAG_NO_COPY_VALUE,
		});
	}

	if (auto r = nghttp2_submit_trailer(_session, stream_id, nv.data(), nv.size()); r != 0) {
		throw std::runtime_error(std::string("Failed to submit trailers: ") + nghttp2_strerror(r));
	}
}
} // namespace detail
} // namespace h2
} // namespace grpcxx
