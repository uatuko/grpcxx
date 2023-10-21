#pragma once

#include <optional>
#include <string>

namespace grpcxx {
namespace detail {
class message {
public:
	void bytes(const std::string_view bytes);

private:
	void parse();

	uint8_t                 _compressed = 0;
	std::optional<uint32_t> _length     = std::nullopt;

	std::string _bytes;
};
} // namespace detail
} // namespace grpcxx
