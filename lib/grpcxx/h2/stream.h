#pragma once

#include <map>
#include <string>
#include <vector>

namespace grpcxx {
namespace h2 {
class stream {
public:
	using data_t    = std::vector<uint8_t>;
	using headers_t = std::map<std::string, std::string>;
	using id_t      = int32_t;

	stream(const id_t &id);

	data_t    data;
	headers_t headers;

	const id_t id() const noexcept { return _id; }

private:
	id_t _id;
};
} // namespace h2
} // namespace grpcxx
