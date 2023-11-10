#pragma once

#include <string>

#include "message.h"
#include "status.h"

namespace grpcxx {
namespace detail {
class response {
public:
	response(int32_t id, status::code_t code = status::code_t::ok) : _id(id), _status(code) {}

	int32_t id() const noexcept { return _id; }

	std::string      bytes() const noexcept { return _msg.bytes(); }
	std::string_view status() noexcept { return _status; }

	void data(std::string &&d) noexcept { _msg = std::move(d); }
	void status(class status &&s) noexcept { _status = std::move(s); }

private:
	int32_t      _id;
	message      _msg;
	class status _status;
};
} // namespace detail
} // namespace grpcxx
