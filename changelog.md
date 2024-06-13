# Changelog

## v0.3.0 - 13th June, 2024

### 🔦 Spotlight
🔥 grpcxx now offers a choice between [libuv](https://libuv.org) and asio ([Asio](https://github.com/chriskohlhoff/asio)
and [Boost Asio](https://github.com/boostorg/asio)) to be used as the I/O library for handling raw TCP
data. The default is to use libuv but asio can be used instead by using the `GRPCXX_USE_ASIO` CMake
option (or pre-processor macro).

### What's Changed
* Asio as an alternative to libuv for I/O and event loop by @uatuko in https://github.com/uatuko/grpcxx/pull/26


**Full Changelog**: https://github.com/uatuko/grpcxx/compare/v0.2.0...v0.3.0


## 0.2.0 - 2nd February, 2024

* Run only one libuv event loop for I/O and move processing to worker threads (#24)

## 0.1.4 - 12th January, 2024

* Fix to ensure idle connections don't block worker threads (#20).

## 0.1.3 - 28th December, 2023

* Fix to map types correctly when generating RPC definitions (#16).

## 0.1.2 - 28th December, 2023

* Fix to include `<cstdint>` for fixed width integer types.

## 0.1.1 - 22nd December, 2023

* Fix `protoc-gen-grpcxx` to output service definitions correctly (#14).

## 0.1.0 - 20th December, 2023

* Initial release.
