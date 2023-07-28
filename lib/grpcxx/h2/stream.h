#pragma once

#include <map>
#include <string>
#include <vector>

#include <nghttp2/nghttp2.h>

namespace grpcxx {
namespace h2 {
class stream {
public:
	using data_t    = std::vector<uint8_t>;
	using headers_t = std::map<std::string, std::string>;
	using id_t      = int32_t;

	stream(const id_t &id, nghttp2_session *session) noexcept;

	data_t    data;
	headers_t headers;

	const id_t id() const noexcept { return _id; }

	void send(const headers_t &headers, const data_t &data) const;

private:
	id_t             _id;
	nghttp2_session *_session;
};
} // namespace h2
} // namespace grpcxx
