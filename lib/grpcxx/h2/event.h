#pragma once

#include "headers.h"

#include <cstdint>
#include <optional>
#include <string_view>

namespace grpcxx {
namespace h2 {
namespace detail {
struct event {
	using header_t = std::optional<class header>;

	enum struct type_t : uint8_t {
		noop = 0,
		stream_close,
		stream_data,
		stream_end,
		stream_header,
	};

	const std::string_view data;
	const int32_t          stream_id = -1;
	const type_t           type      = type_t::noop;

	header_t header = std::nullopt;
};
} // namespace detail
} // namespace h2
} // namespace grpcxx
