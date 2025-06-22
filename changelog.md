# Changelog

## v0.6.2 - 22nd June, 2025

### What's changed

* Make `PROTOBUF_MINVERSION` CMake variable user-settable (https://github.com/uatuko/grpcxx/issues/56)

**Full Changelog**: https://github.com/uatuko/grpcxx/compare/v0.6.1...v0.6.2


## v0.6.1 - 10th January, 2025

### What's Changed
* Avoid defining `BUILD_SHARED_LIBS` cmake variable by @uatuko in https://github.com/uatuko/grpcxx/pull/54

**Full Changelog**: https://github.com/uatuko/grpcxx/compare/v0.6.0...v0.6.1


## v0.6.0 - 7th January, 2025

### 🔦 Spotlight
Server can be started using a pre-initialised socket handler and stopped using a `std::stop_token` (when using libuv).

### What's Changed
* Handle UV server shutdown and allow socket activation by @tchernobog in https://github.com/uatuko/grpcxx/pull/42
* Allow constructing a `grpcxx::server` using an external `uv_loop_t` by @tchernobog in https://github.com/uatuko/grpcxx/pull/44
* Update GitHub workflows by @uatuko in https://github.com/uatuko/grpcxx/pull/43
* Windows builds by @uatuko in https://github.com/uatuko/grpcxx/pull/46
* Make libuv server api semantically similar to asio by @uatuko in https://github.com/uatuko/grpcxx/pull/52
* Fix to ensure length prefixed messages are parsed correctly by @uatuko in https://github.com/uatuko/grpcxx/pull/51
* Update build workflow by @uatuko in https://github.com/uatuko/grpcxx/pull/53

**Full Changelog**: https://github.com/uatuko/grpcxx/compare/v0.5.1...v0.6.0


## v0.5.1 - 28th October, 2024

### What's Changed
* Fix cmake configs by @tchernobog in https://github.com/uatuko/grpcxx/pull/41

**Full Changelog**: https://github.com/uatuko/grpcxx/compare/v0.5.0...v0.5.1


## v0.5.0 - 27th October, 2024

### 🔦 Spotlight
Builds now support linking to system installed dependencies and provide install targets (https://github.com/uatuko/grpcxx/pull/40).

### What's Changed
* Install and export artifacts by @tchernobog in https://github.com/uatuko/grpcxx/pull/40

**Full Changelog**: https://github.com/uatuko/grpcxx/compare/v0.4.0...v0.5.0


## v0.4.0 - 22nd September, 2024

### 🔦 Spotlight
Fix header file include order to help with consistent compilations across different platforms and compilers (https://github.com/uatuko/grpcxx/pull/39).

### What's Changed
* Fix ordering of include files by @tchernobog in https://github.com/uatuko/grpcxx/pull/39

### New Contributors
* @tchernobog

**Full Changelog**: https://github.com/uatuko/grpcxx/compare/v0.3.0...v0.4.0


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
