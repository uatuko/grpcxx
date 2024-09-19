#pragma once

#include "../server_base.h"

#ifdef BOOST_ASIO_STANDALONE
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#else
#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>
#endif

#include <string_view>

namespace grpcxx {
namespace asio {
class server : public ::grpcxx::server_base {
public:
	server(const server &) = delete;
	server()               = default;

	ASIO_NS::awaitable<void> listen(std::string_view ip, int port);
	void                     run(std::string_view ip, int port);

private:
	ASIO_NS::awaitable<void> conn(ASIO_NS::ip::tcp::socket sock);
};
} // namespace asio
} // namespace grpcxx
