#pragma once

#include <memory>
#include <string>

#include <uv.h>

#include "reader.h"
#include "writer.h"

class conn {
public:
	struct deleter {
		void operator()(void *handle) const noexcept {
			uv_close(static_cast<uv_handle_t *>(handle), [](uv_handle_t *handle) {
				delete reinterpret_cast<uv_tcp_t *>(handle);
			});
		}
	};

	conn(uv_stream_t *stream) : _handle(new uv_tcp_t{}, deleter{}) {
		uv_tcp_init(stream->loop, _handle.get());
		_handle->data = this;

		accept(stream);
	}

	operator bool() const noexcept { return _reader; }

	void accept(uv_stream_t *stream) {
		if (auto r = uv_accept(stream, reinterpret_cast<uv_stream_t *>(_handle.get())); r != 0) {
			throw std::runtime_error(
				std::string("Failed to accept for connection: ") + uv_strerror(r));
		}

		if (auto r =
				uv_read_start(reinterpret_cast<uv_stream_t *>(_handle.get()), alloc_cb, read_cb);
			r != 0) {
			throw std::runtime_error(
				std::string("Failed to start reading data: ") + uv_strerror(r));
		}
	}

	reader &read() { return _reader; }

	writer write(std::string_view data) {
		return {std::reinterpret_pointer_cast<uv_stream_t>(_handle), data};
	}

private:
	static void alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
		auto *c = static_cast<conn *>(handle->data);

		auto &buffer = c->_reader.buffer();
		buffer.clear();

		*buf = uv_buf_init(buffer.data(), buffer.capacity());
	}

	static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
		auto *c = static_cast<conn *>(stream->data);
		if (nread < 0) {
			c->_reader.close();
			return;
		}

		c->_reader.buffer().size(nread);
		c->_reader.resume();
	}

	reader                    _reader;
	std::shared_ptr<uv_tcp_t> _handle;
};
