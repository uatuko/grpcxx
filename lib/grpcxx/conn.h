#pragma once

#include <memory>

#include <uv.h>

#include "h2/session.h"

#include "reader.h"
#include "writer.h"

namespace grpcxx {
namespace detail {
class conn {
public:
	using handle_t = std::shared_ptr<uv_tcp_t>;

	conn(uv_stream_t *stream);
	conn(const conn &) = delete;

	h2::session &session() noexcept { return _session; }

	reader read() const noexcept;
	writer write(std::string_view bytes) const noexcept;

private:
	struct deleter {
		void operator()(void *handle) const noexcept {
			uv_close(static_cast<uv_handle_t *>(handle), [](uv_handle_t *handle) {
				delete reinterpret_cast<uv_tcp_t *>(handle);
			});
		}
	};

	handle_t    _handle;
	h2::session _session;
};
} // namespace detail
} // namespace grpcxx
