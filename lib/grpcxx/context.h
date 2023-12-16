#pragma once

#include <string_view>
#include <unordered_map>

namespace grpcxx {
// Forward declarations
namespace detail {
class request;
} // namespace detail

class context {
public:
	using meta_t = std::unordered_map<std::string_view, std::string_view>;

	context()                = default;
	context(context &&)      = default;
	context(const context &) = delete;

	context(const detail::request &req) noexcept;

	meta_t::mapped_type meta(meta_t::key_type key) const noexcept;
	void                meta(const detail::request &req) noexcept;

private:
	meta_t _meta;
};
} // namespace grpcxx
