#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace grpcxx {
namespace h2 {
struct header {
	const std::string name;
	const std::string value;
};

struct header_view {
	std::string_view name;
	std::string_view value;
};

using headers = std::vector<header_view>;
} // namespace h2
} // namespace grpcxx
