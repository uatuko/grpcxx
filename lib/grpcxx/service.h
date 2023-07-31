#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>

#include "status.h"

namespace grpcxx {
namespace concepts {
template <typename T>
concept rpc_type = requires(T t) {
	// Method
	T::method;
	requires std::same_as<std::remove_cv_t<decltype(T::method)>, std::string_view>;

	// Request
	typename T::request_type;
	{ t.map(std::declval<const std::string &>()) } -> std::same_as<typename T::request_type>;

	// Response
	typename T::response_type;
	typename T::optional_response_type;
	{
		t.map(std::declval<const typename T::optional_response_type &>())
		} -> std::same_as<std::string>;

	// Handler
	{t.fn};
	requires std::same_as < decltype(t.fn),
		std::function <
			std::pair<status, typename T::optional_response_type>(const typename T::request_type &)
	>> ;
};
} // namespace concepts

template <concepts::rpc_type... R> class service {
public:
	using data_t     = std::string;
	using response_t = std::pair<status, data_t>;

	using handler_t  = std::function<response_t(const data_t &)>;
	using handlers_t = std::unordered_map<std::string_view, handler_t>;

	constexpr service() {
		std::apply([&](auto &&...args) { (make_handler(args), ...); }, _methods);
	}

	response_t call(std::string_view method, const std::string &data) {
		auto it = _handlers.find(method);
		if (it == _handlers.end()) {
			return {status::code_t::not_found, {}};
		}

		return it->second(data);
	}

	template <fixed_string M> constexpr void rpc(const auto &fn) {
		constexpr bool found = ((R::method == M) || ...);
		static_assert(found, "Method not found in service");

		std::apply([&](auto &&...args) { (rpc_helper<M>(args, fn) || ...); }, _methods);
	}

private:
	constexpr void make_handler(const auto &rpc) {
		auto handler = [&rpc](const data_t &data) -> response_t {
			auto req    = rpc.map(data);
			auto result = rpc.fn(req);

			return {result.first, rpc.map(result.second)};
		};

		_handlers.insert({rpc.method, handler});
	}

	template <fixed_string M> constexpr bool rpc_helper(auto &&rpc, auto &&fn) {
		if constexpr (M == std::remove_reference_t<decltype(rpc)>::method) {
			rpc.fn = fn;
			return true;
		}

		return false;
	}

	handlers_t       _handlers;
	std::tuple<R...> _methods;
};
} // namespace grpcxx
