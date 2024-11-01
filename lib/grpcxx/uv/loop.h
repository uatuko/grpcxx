#pragma once

#include <uv.h>

namespace grpcxx::uv::detail {

class loop_t {
public:
	loop_t();
	explicit loop_t(uv_loop_t &uv_loop);
	loop_t(const loop_t &) = delete;
	loop_t(loop_t &&)      = default;
	~loop_t() noexcept;

	operator uv_loop_t *() noexcept { return _loop; }

	bool is_managed() const { return _managed; }

private:
	bool       _managed;
	uv_loop_t *_loop;
};

} // namespace grpcxx::uv::detail
