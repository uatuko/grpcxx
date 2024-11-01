#include "loop.h"

namespace grpcxx::uv::detail {

loop_t::loop_t() : _managed{true}, _loop{new uv_loop_t{}} {
	uv_loop_init(_loop);
}

loop_t::loop_t(uv_loop_t &loop) : _managed(false), _loop{&loop} {}

loop_t::~loop_t() noexcept {
	if (!is_managed()) {
		return;
	}

	// Ensures all remaining handles are cleaned up (but without any
	// special close callback)
	uv_walk(
		_loop,
		[](uv_handle_t *handle, void *) {
			if (!uv_is_closing(handle)) {
				uv_close(handle, nullptr);
			}
		},
		nullptr);

	while (uv_loop_close(_loop) == UV_EBUSY) {
		uv_run(_loop, UV_RUN_ONCE);
	}

	delete _loop;
}

} // namespace grpcxx::uv::detail
