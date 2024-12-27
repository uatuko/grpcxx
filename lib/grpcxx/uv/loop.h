#pragma once

#include <uv.h>

namespace grpcxx {
namespace uv {
namespace detail {
class loop {
public:
	loop(const loop &) = delete;
	loop();

	~loop() noexcept;

	operator uv_loop_t *() noexcept { return &_loop; }

private:
	uv_loop_t _loop;
};
} // namespace detail
} // namespace uv
} // namespace grpcxx
