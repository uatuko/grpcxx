#include "loop.h"

namespace grpcxx {
namespace uv {
namespace detail {
loop::loop() {
	uv_loop_init(&_loop);
}

loop::~loop() noexcept {
	if (uv_loop_alive(&_loop)) {
		uv_stop(&_loop);
	}

	// Ideally all handles should've been closed by now, do a cleanup just in case
	uv_walk(
		&_loop,
		[](uv_handle_t *handle, void *) {
			if (!uv_is_closing(handle)) {
				uv_close(handle, nullptr);
			}
		},
		nullptr);

	while (uv_loop_close(&_loop) == UV_EBUSY) {
		uv_run(&_loop, UV_RUN_ONCE);
	}
}
} // namespace detail
} // namespace uv
} // namespace grpcxx
