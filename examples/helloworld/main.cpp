#include <cstdio>

#include <grpcxx/server.h>

#include "helloworld/v1/greeter.grpcxx.pb.h"

using namespace helloworld::v1::Greeter;

// Implement rpc application logic using template specialisation for generated `ServiceImpl` struct
template <>
rpcHello::result_type ServiceImpl::call<rpcHello>(
	grpcxx::context &, const GreeterHelloRequest &req) {
	GreeterHelloResponse res;
	res.set_message("Hello `" + req.name() + "` 👋");
	return {grpcxx::status::code_t::ok, res};
}

// Application defined struct implementing rpc application logic
struct GreeterImpl {
	template <typename T>
	typename T::result_type call(grpcxx::context &, const typename T::request_type &) {
		return {grpcxx::status::code_t::unimplemented, std::nullopt};
	}
};

template <>
rpcHello::result_type GreeterImpl::call<rpcHello>(
	grpcxx::context &, const helloworld::v1::GreeterHelloRequest &req) {
	helloworld::v1::GreeterHelloResponse res;
	res.set_message("Hello `" + req.name() + "` 👋");
	return {grpcxx::status::code_t::ok, res};
}

int main() {
	GreeterImpl greeter;
	Service     service(greeter);

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
