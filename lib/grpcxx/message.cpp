#include "message.h"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace grpcxx {
namespace detail {
message::message(std::string &&data) : _data(std::move(data)), _length(data.size()) {
	uint32_t size = _length.value();

	std::array<std::string::value_type, 5> bytes;
	bytes[0] = 0x00;
	bytes[1] = (size >> 24) & 0xff;
	bytes[2] = (size >> 16) & 0xff;
	bytes[3] = (size >> 8) & 0xff;
	bytes[4] = size & 0xff;

	_prefix = {bytes.data(), bytes.size()};
}

std::string message::bytes() const noexcept {
	return _prefix + _data;
}

void message::bytes(std::string_view bytes) {
	if (_length && _data.size() >= _length.value()) {
		throw std::length_error("Message larger than what's indicated in the prefix.");
	}

	std::size_t head = 0;
	if (_prefix.size() < 5) {
		std::size_t n = bytes.size() > 5 ? 5 : bytes.size();
		_prefix.append(bytes.data(), n);

		head += n;
	}

	parse();

	if (bytes.size() - head > 0) {
		_data.append(bytes.data() + head, bytes.size() - head);
	}
}

std::string_view message::data() const noexcept {
	if (!_length || (_data.size() < _length.value())) {
		return {};
	}

	return _data;
}

void message::parse() {
	if (_length) {
		// Already parsed
		return;
	}

	if (_prefix.size() < 5) {
		// Not enough bytes to parse
		return;
	}

	_compressed = _prefix[0];
	_length     = std::make_optional(
        (_prefix[1] << 24) | (_prefix[2] << 16) | (_prefix[3] << 8) | (_prefix[4] & 0xff));

	const std::size_t cap = _length.value() + 5;
	if (_data.capacity() < cap) {
		_data.reserve(cap);
	}
}

std::string_view message::prefix() const noexcept {
	return _prefix;
}
} // namespace detail
} // namespace grpcxx
