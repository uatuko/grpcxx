#pragma once

#include "context.h"
#include "status.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace grpcxx {

// Forward declarations
namespace detail {
class request;
class response;
} // namespace detail

class server_base {
public:
	using fn_t = std::function<std::pair<status, std::string>(
		context &, std::string_view, std::string_view)>;

	using services_t = std::unordered_map<std::string_view, fn_t>;

	template <typename S> void add(S &s) {
		fn_t fn = std::bind_front(&S::call, &s);
		_services.insert({s.name(), fn});
	}

protected:
	detail::response process(const detail::request &req) const noexcept;

private:
	services_t _services;
};
} // namespace grpcxx
