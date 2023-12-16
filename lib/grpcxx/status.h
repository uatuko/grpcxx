#pragma once

#include <string>

namespace grpcxx {
class status {
public:
	enum struct code_t : int8_t {
		ok                  = 0,
		cancelled           = 1,
		unknown             = 2,
		invalid_argument    = 3,
		deadline_exceeded   = 4,
		not_found           = 5,
		already_exists      = 6,
		permission_denied   = 7,
		resource_exhausted  = 8,
		failed_precondition = 9,
		aborted             = 10,
		out_of_range        = 11,
		unimplemented       = 12,
		internal            = 13,
		unavailable         = 14,
		data_loss           = 15,
		unauthenticated     = 16,
	};

	status(code_t code = code_t::ok) : _code(code), _str() {}
	status(code_t code, std::string &&details) :
		_code(code), _details(std::move(details)), _str() {}

	operator std::string_view() const { return str(); }

	code_t code() const noexcept { return _code; }

	const std::string &details() const noexcept { return _details; }

	std::string_view str() const {
		if (_str.empty()) {
			_str = std::to_string(static_cast<int8_t>(_code));
		}

		return _str;
	}

private:
	code_t      _code;
	std::string _details;

	mutable std::string _str;
};
} // namespace grpcxx
