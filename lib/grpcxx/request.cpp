#include "request.h"

namespace grpcxx {
namespace detail {
request::request(int32_t id) : _id(id) {}

void request::header(const std::string &name, const std::string &value) noexcept {
	// Avoid processing further if the request is already invalid
	if (invalid()) {
		return;
	}

	if (name == ":method") {
		flag(flags_t::header_method);

		if (value != "POST") {
			flag(flags_t::invalid);
		}

		return;
	}

	if (name == ":path") {
		flag(flags_t::header_path);

		if (value.front() != '/') {
			flag(flags_t::invalid);
			return;
		}

		const auto n = value.find('/', 1);
		if (n == std::string::npos) {
			flag(flags_t::invalid);
			return;
		}

		_service = value.substr(1, n - 1);
		_method  = value.substr(n + 1);
		if (_method.empty()) {
			flag(flags_t::invalid);
			return;
		}

		return;
	}

	if (name == "content-type") {
		flag(flags_t::header_content_type);

		if (!value.starts_with("application/grpc")) {
			flag(flags_t::invalid);
		}

		return;
	}
}

bool request::invalid() const noexcept {
	return flag(flags_t::invalid);
}
} // namespace detail
} // namespace grpcxx
