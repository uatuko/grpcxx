#pragma once

#include <string>
#include <vector>

namespace grpcxx {
namespace h2 {
struct header {
	const std::string name;
	const std::string value;
};

using headers = std::vector<header>;
} // namespace h2
} // namespace grpcxx
