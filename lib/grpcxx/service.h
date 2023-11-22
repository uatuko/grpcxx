#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>

#include "fixed_string.h"
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
	{ t.map(std::declval<std::string_view>()) } -> std::same_as<typename T::request_type>;

	// Response
	typename T::response_type;
	typename T::optional_response_type;
	{
		t.map(std::declval<const typename T::optional_response_type &>())
	} -> std::same_as<std::string>;

	// Result
	typename T::result_type;
	requires std::same_as<
		typename T::result_type,
		std::pair<grpcxx::status, typename T::optional_response_type>>;
};
} // namespace concepts

template <fixed_string N, concepts::rpc_type... R> class service {
public:
	using response_t = std::pair<status, std::string>;

	using handler_t  = std::function<response_t(std::string_view)>;
	using handlers_t = std::unordered_map<std::string_view, handler_t>;

	template <typename I> constexpr service(I &impl) {
		std::apply(
			[&](auto &&...args) {
				auto helper = [&](const auto &rpc) {
					auto handler = [&impl, &rpc](std::string_view data) -> response_t {
						using type = std::remove_cvref_t<decltype(rpc)>;

						auto req    = rpc.map(data);
						auto result = std::invoke(&I::template call<type>, impl, req);

						return {result.first, rpc.map(result.second)};
					};

					_handlers.insert({rpc.method, handler});
				};

				(helper(args), ...);
			},
			std::tuple<R...>());
	}

	response_t call(std::string_view method, std::string_view data) {
		auto it = _handlers.find(method);
		if (it == _handlers.end()) {
			return {status::code_t::not_found, {}};
		}

		return it->second(data);
	}

	constexpr std::string_view name() const noexcept { return {N}; }

private:
	handlers_t _handlers;
};
} // namespace grpcxx
