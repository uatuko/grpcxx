#include <cstdio>
#include <map>
#include <string>

#include <grpcxx/rpc.h>
#include <grpcxx/server.h>
#include <grpcxx/service.h>

#include "helloworld/v1/greeter.pb.h"

// ---- [start] generated code ---- //
namespace helloworld {
namespace v1 {
namespace Greeter {
using rpcHello =
	grpcxx::rpc<"Hello", helloworld::v1::GreeterHelloRequest, helloworld::v1::GreeterHelloResponse>;

using Service = grpcxx::service<"helloworld.v1.Greeter", rpcHello>;

struct ServiceImpl {
	template <typename T> typename T::result_type call(const typename T::request_type &) {
		return {grpcxx::status::code_t::unimplemented, std::nullopt};
	}
};
}; // namespace Greeter
} // namespace v1
} // namespace helloworld
// ---- [end] generated code ---- //

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
