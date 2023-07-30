#pragma once

#include "fixed_string.h"

namespace grpcxx {
template <fixed_string M, typename T, typename U> struct rpc {
	static constexpr std::string_view method{M};
	using request_type  = T;
	using response_type = U;
};
} // namespace grpcxx
