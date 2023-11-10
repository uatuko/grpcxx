#include "conn.h"

namespace grpcxx {
namespace detail {
conn::conn(uv_tcp_t *handle) noexcept :
	_buf(), _eos(false), _h(nullptr), _handle(handle), _reqs(), _session(), _streams() {
	_handle->data = this;

	uv_read_start(
		reinterpret_cast<uv_stream_t *>(_handle),
		[](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
			auto *c = static_cast<conn *>(handle->data);
			*buf    = uv_buf_init(c->_buf.data(), c->_buf.capacity());
		},
		[](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
			if (nread <= 0) {
				if (nread < 0) {
					uv_close(reinterpret_cast<uv_handle_t *>(stream), close_cb);
				}

				return;
			}

			auto *c = static_cast<conn *>(stream->data);

			try {
				c->read(nread);
			} catch (...) {
				uv_close(reinterpret_cast<uv_handle_t *>(stream), close_cb);
				return;
			}
		});
}

conn::requests_t conn::await_resume() noexcept {
	_h = nullptr;

	if (_reqs.empty()) {
		return {};
	}

	auto reqs = std::move(_reqs);
	_reqs     = requests_t();

	return reqs;
}

void conn::close_cb(uv_handle_t *handle) {
	auto *c = static_cast<conn *>(handle->data);
	c->_eos = true;
	c->resume();
}

void conn::read(std::size_t n) {
	for (const auto &ev : _session.read({_buf.data(), n})) {
		if (ev.stream_id <= 0) {
			continue;
		}

		if (ev.type == h2::event::type_t::stream_close) {
			_streams.erase(ev.stream_id);
			continue;
		}

		streams_t::iterator it  = _streams.emplace(ev.stream_id, ev.stream_id).first;
		auto               &req = it->second;

		switch (ev.type) {
		case h2::event::type_t::stream_data: {
			req.read(ev.data);
			break;
		}

		case h2::event::type_t::stream_end: {
			_reqs.push_front(std::move(req));
			_streams.erase(ev.stream_id);
			break;
		}

		case h2::event::type_t::stream_header: {
			req.header(ev.header->name, ev.header->value);
			break;
		}

		default:
			break;
		}
	}

	write();

	if (!_reqs.empty()) {
		resume();
	}
}

void conn::resume() noexcept {
	if (_h) {
		_h.resume();
	}
}

void conn::write() noexcept {
	auto *bytes = new std::string();
	for (auto chunk = _session.pending(); chunk.size() > 0; chunk = _session.pending()) {
		bytes->append(chunk);
	}

	if (bytes->empty()) {
		return;
	}

	auto  buf = uv_buf_init(const_cast<char *>(bytes->data()), bytes->size());
	auto *req = new uv_write_t();
	req->data = bytes;

	if (auto r = uv_write(req, reinterpret_cast<uv_stream_t *>(_handle), &buf, 1, write_cb);
		r != 0) {
		// FIXME: error handling
		delete bytes;
		delete req;
	}
}

void conn::write(response resp) noexcept {
	_session.headers(
		resp.id(),
		{
			{":status", "200"},
			{"content-type", "application/grpc"},
		});

	_session.data(resp.id(), resp.bytes());
	write();

	_session.trailers(
		resp.id(),
		{
			{"grpc-status", resp.status()},
		});

	write();
}

void conn::write_cb(uv_write_t *req, int status) {
	if (status != 0) {
		// TODO: error handling
	}

	auto *str = static_cast<std::string *>(req->data);
	delete str;

	delete req;
}
} // namespace detail
} // namespace grpcxx
