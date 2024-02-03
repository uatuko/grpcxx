#pragma once

#include <forward_list>
#include <unordered_map>

#include <asio/awaitable.hpp>
#include <asio/ip/tcp.hpp>

#include "../h2/session.h"
#include "../request.h"
#include "../response.h"

namespace grpcxx {
namespace asio {
namespace detail {
class conn {
public:
	using requests_t = std::forward_list<::grpcxx::detail::request>;
	using streams_t  = std::unordered_map<int32_t, ::grpcxx::detail::request>;

	conn(const conn &) = delete;
	conn(::asio::ip::tcp::socket &&sock) noexcept;

	operator bool() const noexcept { return !_eos; }

	::asio::awaitable<requests_t> reqs() noexcept;
	::asio::awaitable<void>       write(::grpcxx::detail::response resp) noexcept;

private:
	template <std::size_t N> class buffer_t {
	public:
		constexpr char       *data() noexcept { return &_data[0]; }
		constexpr std::size_t capacity() const noexcept { return N; }

	private:
		char _data[N];
	};

	requests_t              read(std::size_t n);
	::asio::awaitable<void> write();

	buffer_t<1024> _buf; // FIXME: make size configurable
	bool           _eos = false;
	h2::session    _session;
	streams_t      _streams;

	::asio::ip::tcp::socket _sock;
};
} // namespace detail
} // namespace asio
} // namespace grpcxx
