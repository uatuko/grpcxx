#pragma once

#include "stream.h"

namespace grpcxx {
namespace h2 {
enum struct event_type {
	eos = 0, // end of stream
	data,
	headers,
};

struct event {
	std::shared_ptr<stream> stream;
	const event_type        type;
};
} // namespace h2
} // namespace grpcxx
