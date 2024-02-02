# grpcxx

[![license](https://img.shields.io/badge/license-MIT-green)](https://raw.githubusercontent.com/uatuko/grpcxx/main/LICENSE)
[![build](https://github.com/uatuko/grpcxx/actions/workflows/build.yaml/badge.svg?branch=main)](https://github.com/uatuko/grpcxx/actions/workflows/build.yaml)
[![release](https://img.shields.io/github/v/release/uatuko/grpcxx)](https://github.com/uatuko/grpcxx/releases)

🚀 Blazing fast gRPC C++ Server implemented using modern standards (C++20).

## Features

* Fast - More than 2x faster compared to the official implementation(s).
* Simple - Overall smaller codebase and 95% less generated code compared to the official implementation.
* Flexible - The application code is given greater implementation choice by using C++ concepts instead of being restricted to one choice.


## Benchmarks

> You can find more detailed benchmark results in https://github.com/uatuko/grpcxx/issues/25 and https://github.com/uatuko/grpcxx/issues/21#issuecomment-1890901856.

|                                      | 1a  | 1b   | 2a   | 2b   | 3a   | 3b   |
| ------------------------------------ | --- | ---- | ---- | ---- | ---- | ---- |
| gRPC v1.48.4 (callback)              | 25k | 87k  | 76k  | **152k** | 96k  | 142k |
| grpc-go v1.56.2                      | 27k | 103k | 94k  | 191k | 90k  | **308k** |
| Rust (tonic v0.10.2)                 | 29k | 66k  | 95k  | 176k | 68k  | **212k** |
| grpcxx v0.2.0 (single-threaded)      | 42k | 193k | 120k | 449k | 160k | **452k** |
| grpcxx v0.2.0 (2 workers)            | 33k | 166k | 120k | **431k** | 91k  | 425k |
| grpcxx v0.2.0 (hardware concurrency) | 34k | 164k | 118k | **450k** | 101k | 408k |


## Documentation

You can find a bit more detailed documentation in [docs/](docs/README.md).


## Getting started

### Prerequisites

* [CMake](https://cmake.org) (>= 3.23)
* [Protobuf](https://protobuf.dev) (>= 3.15)

### Installing

There aren't any installable packages at this stage but you can add this to your CMake project as a dependency using
[FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html).

e.g.
```cmake
# grpcxx
FetchContent_Declare(grpcxx
  URL      https://github.com/uatuko/grpcxx/archive/refs/tags/v0.1.3.tar.gz
  URL_HASH SHA256=441ca21bed3c0413623440c1608da44e60931631af1dc609c18e4a955f8cb3a5
)
FetchContent_MakeAvailable(grpcxx)
```

### Using grpcxx

> 💡 You can find a complete _helloworld_ example [here](https://github.com/uatuko/grpcxx/tree/main/examples/helloworld).

In order to use grpcxx (similar to other gRPC implementations) you'll need to;

1. Generate code
2. Implement RPC endpoints
3. Run a server to process RPC requests

#### Code generation

You can use the `protoc-gen-grpcxx` Protobuf compiler plugin to generate the Protobuf and gRPC server code.

e.g.
```
❯ protoc --proto_path=. \
  --cpp_out=/path/to/output \
  --grpcxx_out=/path/to/output \
  --plugin=/path/to/protoc-gen-grpcxx \
  helloworld/v1/greeter.proto
```

This will generate 3 files, which you'll need to compile and include in your build.
```
❯ tree /path/to/output
/path/to/output
└── helloworld
  └── v1
    ├── greeter.grpcxx.pb.h
    ├── greeter.pb.cc
    └── greeter.pb.h
```

#### Implementing RPC endpoints

Code generation only creates a stub service to return `UNIMPLEMENTED` gRPC status responses for all the endpoints. You
will need to add your own implementation to return something more meaningful.

One way to do this is by specialising the generated `ServiceImpl` struct.

e.g.
```cpp
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
```

> 💡 The generated `ServiceImpl` struct is a very simple struct with 6 lines of code. You can easily create your own
instead of having to use the generated code giving you more flexibility to structure your code in a way that works best
for you. An example of this can be found [here](https://github.com/uatuko/grpcxx/blob/c6934c3223a76f50439bb1dda98aa25482829b95/examples/helloworld/main.cpp#L19).

#### Running a server

e.g.
```cpp
helloworld::v1::Greeter::ServiceImpl greeter; // Instance of the RPC implementation
helloworld::v1::Greeter::Service     service(greeter); // Service (using the RPC implementation)

grpcxx::server server; // Server instance
server.add(service); // Add the service to the server instance

std::printf("Listening on [127.0.0.1:7000] ...\n");
server.run("127.0.0.1", 7000); // Listen and serve
```
