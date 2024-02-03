#include <cstdio>

#include <google/rpc/code.pb.h>
#include <google/rpc/status.pb.h>
#include <grpcxx/server.h>

#include "examples/v1/errors.grpcxx.pb.h"

namespace b64 {
static constexpr char alphabet[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string encode(std::string_view in) {
	std::string out;
	out.reserve(4 * ((in.size() + 2) / 3)); // Encoding 3 bytes will result in 4 bytes

	int i = 0, j = -6;
	for (char c : in) {
		i  = (i << 8) + c;
		j += 8;

		while (j >= 0) {
			out.push_back(alphabet[(i >> j) & 0x3f]);
			j -= 6;
		}
	}

	if (j > -6) {
		out.push_back(alphabet[((i << 8) >> (j + 8)) & 0x3f]);
	}

	while (out.size() % 4) {
		out.push_back('=');
	}

	return out;
}
} // namespace b64

using namespace examples::v1::Errors;
template <>
rpcTest::result_type ServiceImpl::call<rpcTest>(
	grpcxx::context &ctx, const rpcTest::request_type &req) {
	// Use google.rpc.Status to set detailed error response
	google::rpc::Status status;
	status.set_code(google::rpc::INVALID_ARGUMENT);
	status.set_message("Sample error message");

	// Serialise google.rpc.Status
	auto data = status.SerializeAsString();

	// base64 encode the serialised binary data
	auto encoded = b64::encode(data);

	// Construct a grpcxx::status with a status code and the b64 encoded error details
	grpcxx::status s(static_cast<grpcxx::status::code_t>(status.code()), std::move(encoded));

	return {s, std::nullopt};
}

int main() {
	ServiceImpl impl;
	Service     service(impl);

#ifndef GRPCXX_USE_ASIO
	grpcxx::server server;
#else
	asio::io_context ctx;
	grpcxx::server   server(ctx);
#endif
	server.add(service);

	std::printf("Listening on [127.0.0.1:7000] ...\n");
	server.run("127.0.0.1", 7000);

	return 0;
}
