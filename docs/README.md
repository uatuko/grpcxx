# 📜 Documentation

## Table of contents

* [Async I/O](#async-io)
* [API Documentation](api.md)
* [Protobuf compiler plugin](protoc-gen-grpcxx.md)
* Examples
  * [Hello, World! 👋](https://github.com/uatuko/grpcxx/tree/main/examples/helloworld)
  * [Rich error responses](https://github.com/uatuko/grpcxx/tree/main/examples/errors)


## Async I/O

grpcxx offers the choice between [libuv](https://libuv.org) and asio ([Asio](https://github.com/chriskohlhoff/asio)
and [Boost Asio](https://github.com/boostorg/asio)) to be used as the I/O library for handling raw TCP data.

For most use cases the default libuv option should yeild the best results. Asio can be used instead of
libuv by compiling with `GRPCXX_USE_ASIO` CMake option (or pre-processor macro).
