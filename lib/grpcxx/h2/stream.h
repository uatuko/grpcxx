#pragma once

#include <map>
#include <string>

#include <nghttp2/nghttp2.h>

namespace grpcxx {
namespace h2 {
class stream {
public:
	using data_t    = std::string;
	using headers_t = std::map<std::string, std::string>;
	using id_t      = int32_t;

	stream(const id_t &id, nghttp2_session *session) noexcept;

	data_t    buffer;
	data_t    data;
	headers_t headers;

	const id_t id() const noexcept { return _id; }

	void send(const data_t &data);
	void send(const headers_t &headers, bool eos = false) const;

private:
	static ssize_t read_cb(
		nghttp2_session *session, int32_t stream_id, uint8_t *buf, size_t length,
		uint32_t *data_flags, nghttp2_data_source *source, void *);

	id_t             _id;
	nghttp2_session *_session;
};
} // namespace h2
} // namespace grpcxx
