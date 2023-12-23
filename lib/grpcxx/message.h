#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace grpcxx {
namespace detail {
class message {
public:
	message() = default;
	message(std::string &&data);

	std::string bytes() const noexcept;
	void        bytes(std::string_view bytes);

	std::string_view data() const noexcept;
	std::string_view prefix() const noexcept;

private:
	void parse();

	uint8_t                 _compressed = 0;
	std::optional<uint32_t> _length     = std::nullopt;

	std::string _data;
	std::string _prefix;
};
} // namespace detail
} // namespace grpcxx
