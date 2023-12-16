#pragma once

#include <string>
#include <unordered_map>

#include "message.h"

namespace grpcxx {
namespace detail {
class request {
public:
	using metadata_t = std::unordered_map<std::string, std::string>;

	request(request &&)      = default;
	request(const request &) = delete;
	request(int32_t id);

	operator bool() const noexcept;

	int32_t id() const noexcept { return _id; }

	std::string_view data() const noexcept { return _msg.data(); }

	const metadata_t &metadata() const noexcept { return _metadata; }

	const std::string &method() const noexcept { return _method; }
	const std::string &service() const noexcept { return _service; }

	void header(std::string &&name, std::string &&value) noexcept;

	bool invalid() const noexcept;

	void read(const std::string_view data) noexcept;

private:
	// FIXME: would it be better to use std::byte instead?
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

	metadata_t _metadata;
	message    _msg;

	std::string _method;
	std::string _service;
};
} // namespace detail
} // namespace grpcxx
