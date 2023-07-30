#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <tuple>

#include "status.h"

namespace grpcxx {
namespace concepts {
template <typename T>
concept rpc_type = requires(T) {
	T::method;
	{std::same_as<decltype(T::method), std::string_view>};
	typename T::request_type;
	typename T::response_type;
};
} // namespace concepts

template <concepts::rpc_type... R> class service {
public:
	using response_t = std::pair<status, std::string>;

	response_t call(std::string_view method, const std::string &data) {
		response_t response(status::code_t::unimplemented, {});
		dispatch([&](const auto &arg) -> bool {
			if (method != arg.method) {
				return false;
			}

			using rpc_t = std::remove_reference_t<decltype(arg)>;
			typename rpc_t::request_type req;
			if (!req.ParseFromString(data)) {
				throw std::runtime_error("Failed to parse proto data");
			}

			auto res       = rpc<typename rpc_t::request_type, typename rpc_t::response_type>(req);
			response.first = res.first;

			if (res.second) {
				std::string out;
				if (!res.second->SerializeToString(&out)) {
					throw std::runtime_error("Failed to serialise proto data");
				}

				response.second = out;
			}

			return true;
		});

		return response;
	}

	template <typename T, typename U> std::pair<status, std::optional<U>> rpc(const T &t) {
		return {status::code_t::unimplemented, std::nullopt};
	}

private:
	template <typename F> bool dispatch(F &&f) {
		return std::apply(
			[&f](const auto &...args) { return (bool{std::forward<F>(f)(args)} || ...); },
			_methods);
	}

	std::tuple<R...> _methods;
};
} // namespace grpcxx
