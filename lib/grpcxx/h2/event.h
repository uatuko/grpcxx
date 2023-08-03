#pragma once

#include "stream.h"

namespace grpcxx {
namespace h2 {
struct event {
	enum struct type_t : uint8_t {
		reserved = 0,
		stream_data,
		stream_end,
		stream_headers,
	};

	std::shared_ptr<stream> stream;
	const type_t            type;
};
} // namespace h2
} // namespace grpcxx
