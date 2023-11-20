#include <cstdio>

#include <grpcxx/server.h>

#include "helloworld/v1/greeter.grpcxx.pb.h"

using namespace helloworld::v1::Greeter;

// Implement rpc application logic using template specialisation for generated `ServiceImpl` struct
template <> rpcHello::result_type ServiceImpl::call<rpcHello>(const GreeterHelloRequest &req) {
	GreeterHelloResponse res;
	res.set_message("Hello `" + req.name() + "` 👋");
	return {grpcxx::status::code_t::ok, res};
}

// Application defined struct implementing rpc application logic
struct GreeterImpl {
	template <typename T> typename T::result_type call(const typename T::request_type &) {
		return {grpcxx::status::code_t::unimplemented, std::nullopt};
	}

	template <>
	rpcHello::result_type call<rpcHello>(const helloworld::v1::GreeterHelloRequest &req) {
		helloworld::v1::GreeterHelloResponse res;
		res.set_message("Hello `" + req.name() + "` 👋");
		return {grpcxx::status::code_t::ok, res};
	}
};

int main() {
	GreeterImpl greeter;
	Service     service(greeter);

	grpcxx::server server;
	server.add(service);

	std::printf("Listening on [127.0.0.1:7000] ...\n");
	server.run("127.0.0.1", 7000);

	return 0;
}
