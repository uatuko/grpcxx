#pragma once

#include <functional>
#include <optional>
#include <string>

#include "fixed_string.h"
#include "status.h"

namespace grpcxx {
template <fixed_string M, typename T, typename U> struct rpc {
	static constexpr std::string_view method{M};

	using method_type   = fixed_string_t<M>;
	using request_type  = T;
	using response_type = U;

	using optional_response_type = std::optional<response_type>;
	using result_type            = std::pair<status, optional_response_type>;

	request_type map(std::string_view data) const {
		constexpr bool can_map = requires(request_type t) {
			{
				t.ParseFromArray(std::declval<const char *>(), std::declval<size_t>())
			} -> std::same_as<bool>;
		};
		static_assert(can_map, "No known method to deserialize data");

		request_type req;

		if (!req.ParseFromArray(data.data(), data.size())) {
			throw std::runtime_error("Failed to deserialize data");
		}

		return req;
	}

	std::string map(const optional_response_type &res) const {
		constexpr bool can_map = requires(response_type t) {
			{ t.SerializeToString(std::declval<std::string *>()) } -> std::same_as<bool>;
		};
		static_assert(can_map, "No known method to serialize data");

		if (!res) {
			return {};
		}

		std::string data;
		if (!res->SerializeToString(&data)) {
			throw std::runtime_error("Failed to serialize data");
		}

		return data;
	}
};
} // namespace grpcxx
