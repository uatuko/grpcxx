#pragma once

#include <string>

#include "message.h"

namespace grpcxx {
namespace detail {
class request {
public:
	request(int32_t id);

	const std::string &method() const noexcept { return _method; }
	const std::string &service() const noexcept { return _service; }

	void header(const std::string &name, const std::string &value) noexcept;

	bool invalid() const noexcept;

	void recv(const std::string_view data) noexcept;

private:
	enum struct flags_t : uint8_t {
		invalid             = 0x01,
		header_method       = 0x01 << 1, // :method
		header_path         = 0x01 << 2, // :path
		header_content_type = 0x01 << 3, // content-type
	};

	void flag(flags_t f) noexcept { _flags |= static_cast<uint8_t>(f); }
	bool flag(flags_t f) const noexcept {
		return ((_flags & static_cast<uint8_t>(f)) == static_cast<uint8_t>(f));
	}

	uint8_t _flags = 0x00;
	int32_t _id;

	message _msg;

	std::string _method;
	std::string _service;
};
} // namespace detail
} // namespace grpcxx