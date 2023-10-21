#include "message.h"

namespace grpcxx {
namespace detail {
void message::bytes(const std::string_view bytes) {
	if (_length && _bytes.size() >= _length.value() + 5) {
		throw std::length_error("Message larger than what's indicated in the prefix.");
	}

	_bytes.append(bytes);
	parse();
}

void message::parse() {
	if (_length) {
		// Already parsed
		return;
	}

	if (_bytes.size() < 5) {
		// Not enough bytes to parse
		return;
	}

	_compressed = _bytes[0];
	_length =
		std::make_optional((_bytes[1] << 24) | (_bytes[2] << 16) | (_bytes[3] << 8) | (_bytes[4]));

	const std::size_t cap = _length.value() + 5;
	if (_bytes.capacity() < cap) {
		_bytes.reserve(cap);
	}
}
} // namespace detail
} // namespace grpcxx
