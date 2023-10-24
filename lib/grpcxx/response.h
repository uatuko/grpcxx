#pragma once

#include <string>

#include "message.h"
#include "status.h"

namespace grpcxx {
namespace detail {
class response {
public:
	response(status::code_t code = status::code_t::ok) : _status(code) {}

	std::string      bytes() const noexcept { return _msg.bytes(); }
	std::string_view status() noexcept { return _status; }

	void data(std::string &&d) noexcept { _msg = std::move(d); }
	void status(class status &&s) noexcept { _status = std::move(s); }

private:
	class status _status;
	message      _msg;
};
} // namespace detail
} // namespace grpcxx
