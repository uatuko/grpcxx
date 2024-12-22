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
}
} // namespace detail
} // namespace uv
} // namespace grpcxx
