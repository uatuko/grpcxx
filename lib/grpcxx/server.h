#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include "context.h"
#include "status.h"

namespace grpcxx {
// Forward declarations
namespace detail {
class response;
} // namespace detail

class server {
public:
	using fn_t = std::function<std::pair<status, std::string>(
		context &, std::string_view, std::string_view)>;

	using services_t = std::unordered_map<std::string_view, fn_t>;

	server(const server &) = delete;
	server(std::size_t n = std::thread::hardware_concurrency()) noexcept;

	template <typename S> void add(S &s) {
		fn_t fn = std::bind_front(&S::call, &s);
		_services.insert({s.name(), fn});
	}

	const auto &services() const noexcept { return _services; }

	void run(const std::string_view ip, int port);

private:
	asio::awaitable<void> conn(asio::ip::tcp::socket sock);
	detail::response      process(const detail::request &req) const noexcept;

	asio::io_context _ctx;
	services_t       _services;
};
} // namespace grpcxx
