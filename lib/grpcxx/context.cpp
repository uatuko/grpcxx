#include "context.h"

#include "request.h"

namespace grpcxx {
context::context(const detail::request &req) noexcept {
	for (const auto &[key, value] : req.metadata()) {
		_meta.emplace(key, value);
	}
}

context::meta_t::mapped_type context::meta(meta_t::key_type key) const noexcept {
	const auto it = _meta.find(key);
	if (it == _meta.end()) {
		return {};
	}

	return it->second;
}
} // namespace grpcxx
