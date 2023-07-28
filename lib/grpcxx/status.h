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

	status(code_t code = code_t::ok) : _code(code) {}

	operator std::string() const { return std::to_string(static_cast<int8_t>(_code)); };

	code_t code() const noexcept { return _code; }

private:
	code_t _code;
};
} // namespace grpcxx
