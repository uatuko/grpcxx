#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "context.h"
#include "status.h"

namespace grpcxx {
namespace detail {

// Forward declarations
class request;
class response;

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
	response process(const request &req) const noexcept;

private:
	services_t _services;
};
} // namespace detail
} // namespace grpcxx
