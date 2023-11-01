#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "headers.h"

namespace grpcxx {
namespace h2 {
struct event {
	enum struct type_t : uint8_t {
		noop = 0,
		stream_close,
		stream_data,
		stream_end,
		stream_header,
	};

	const std::string_view           data;
	const std::optional<std::string> error;
	const std::optional<header>      header    = std::nullopt;
	const int32_t                    stream_id = -1;
	const type_t                     type      = type_t::noop;
};
} // namespace h2
} // namespace grpcxx
