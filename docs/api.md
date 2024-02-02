# API Documentation

> ⚠️ `grpcxx::detail` namespace is considered internal and may change without any warning.

- [`grpcxx::rpc`](#grpcxxrpc)
  - [Template parameters](#template-parameters)
  - [Example](#example)
- [`grpcxx::service`](#grpcxxservice)
  - [Template parameters](#template-parameters-1)
  - [Example](#example-1)
- [`grpcxx::server`](#grpcxxserver)
  - [Member functions](#member-functions)
    - [(constructor)](#constructor)
    - [add](#add)
    - [run](#run)
  - [Example](#example-2)
- [`grpcxx::context`](#grpcxxcontext)
  - [Member types](#member-types)
  - [Member functions](#member-functions-1)
    - [meta](#meta)
- [`grpcxx::status`](#grpcxxstatus)
  - [Member types](#member-types-1)
  - [Member functions](#member-functions-2)
    - [(constructor)](#constructor-1)
    - [operator std::string\_view](#operator-stdstring_view)
    - [code](#code)
    - [str](#str)
  - [Example](#example-3)


## `grpcxx::rpc`

Defined in header [`<grpcxx/rpc.h>`](https://github.com/uatuko/grpcxx/blob/main/lib/grpcxx/rpc.h).

```cpp
template <
  fixed_string M,
  typename T,
  typename U
> struct rpc;
```

`grpcxx::rpc` template class holds the definition for an RPC method within a gRPC service.

### Template parameters

|||
----- | -----------------
**M** | Method name
**T** | Request message type
**U** | Response message type

`T` and `U` are typically Protobuf generated types but they don't have to be. As long as `T` can be
[constructed from a `const char *`](https://github.com/uatuko/grpcxx/blob/c6934c3223a76f50439bb1dda98aa25482829b95/lib/grpcxx/rpc.h#L25)
and `U` can be [converted to a `std::string`](https://github.com/uatuko/grpcxx/blob/c6934c3223a76f50439bb1dda98aa25482829b95/lib/grpcxx/rpc.h#L42)
any type would work.

### Example

```proto
service Greeter {
  rpc Hello(GreeterHelloRequest) returns (GreeterHelloResponse) {}
}
```

```cpp
using rpcHello = grpcxx::rpc<"Hello", GreeterHelloRequest, GreeterHelloResponse>;
```


## `grpcxx::service`

Defined in header [`<grpcxx/service.h>`](https://github.com/uatuko/grpcxx/blob/main/lib/grpcxx/service.h).

```cpp
template <
  fixed_string N,
  concepts::rpc_type... R
> class service;
```

`grpcxx::service` template class holds the definition for a gRPC service.

### Template parameters

|||
-------- | -----------------
**N**    | Service name
**R...** | RPC methods within the service

**R..** could be types defined using `grpcxx::rpc` template class or any other type that conforms to
[`grpcxx::concepts::rpc_type`](https://github.com/uatuko/grpcxx/blob/c6934c3223a76f50439bb1dda98aa25482829b95/lib/grpcxx/service.h#L16).

### Example

```proto
service Greeter {
  rpc Hello(GreeterHelloRequest) returns (GreeterHelloResponse) {}
}
```

```cpp
using rpcHello = grpcxx::rpc<"Hello", GreeterHelloRequest, GreeterHelloResponse>;

using Service = grpcxx::service<"Greeter", rpcHello>;
```


## `grpcxx::server`

Defined in header [`<grpcxx/server.h>`](https://github.com/uatuko/grpcxx/blob/main/lib/grpcxx/server.h).

```cpp
class server;
```

A gRPC server capable of serving multiple clients.

### Member functions

#### (constructor)

|||
----------------------------------------------------------------------- | ---
`server(std::size_t n = std::thread::hardware_concurrency()) noexcept;` | (1)

Constructs a new server instance.

1. Constructs a server with `n` worker threads (in _addition_ to the I/O thread). If `n` is `0`, all requests will be processed in the I/O thread.

> 💡 Number of workers in the pool can impact throughput and should be tuned for your use-case (i.e. more workers will not always increase throughput).

#### add

|||
--------------------------------------- | ---
`template <typename S> void add(S &s);` | (1)

Add a gRPC service implementation to the server.

1. Add a gRPC service implemented by `s`. `s` must stay in scope while the server in running. If two services are added
with the same name, the second will be ignored.

#### run

|||
------------------------------------------------- | ---
`void run(const std::string_view &ip, int port);` | (1)

Listen and serve incoming gRPC requests.

1. Start listening on `ip` and `port` for incoming gRPC connections and serve requests.

### Example

```cpp
ServiceImpl impl;
Service     service(impl);

grpcxx::server server(2);
server.add(service);

server.run("127.0.0.1", 7000);
```


## `grpcxx::context`

Defined in header [`<grpcxx/context.h>`](https://github.com/uatuko/grpcxx/blob/main/lib/grpcxx/context.h).

```cpp
class context;
```

`grpcxx::context` class carry additional contextual data relevant for gRPC requests.

### Member types

| Type     | Definition                                                |
| -------- | --------------------------------------------------------- |
| `meta_t` | `std::unordered_map<std::string_view, std::string_view>;` |

### Member functions

#### meta

|||
---------------------------------------------------------------- | ---
`meta_t::mapped_type meta(meta_t::key_type key) const noexcept;` | (1)

Retrieve gRPC custom metadata.

1. Retrieve gRPC custom metadata value for key `key`. If there's no value found for the specified key, an empty value
is returned.

> 💡 _Custom Metadata_ are essentially HTTP/2 headers that doesn't start with `:` or `grpc-`.


## `grpcxx::status`

Defined in header [`<grpcxx/status.h>`](https://github.com/uatuko/grpcxx/blob/main/lib/grpcxx/status.h).

```cpp
class status;
```

`grpcxx::status` class represents a gRPC response status.

### Member types

| Type     | Definition                                                |
| -------- | --------------------------------------------------------- |
| `code_t` | `enum struct code_t : int8_t`                             |

```cpp
enum struct code_t : int8_t {
  ok                  = 0,
  cancelled           = 1,
  unknown             = 2,
  invalid_argument    = 3,
  deadline_exceeded   = 4,
  not_found           = 5,
  already_exists      = 6,
  permission_denied   = 7,
  resource_exhausted  = 8,
  failed_precondition = 9,
  aborted             = 10,
  out_of_range        = 11,
  unimplemented       = 12,
  internal            = 13,
  unavailable         = 14,
  data_loss           = 15,
  unauthenticated     = 16,
};
```

### Member functions

#### (constructor)

|||
-------------------------------------------- | ---
`status(code_t code = code_t::ok)`           | (1)
`status(code_t code, std::string &&details)` | (2)

1. Construct a status with an optional status code `code`.
2. Construct a status with code `code` and a base64 encoded `details`. Although it's not in official gRPC documentation
(yet), `details` should be base64 encoded serialised value of a [`google.rpc.Status`](https://github.com/googleapis/googleapis/blob/db5ce67d735d2ceb6fe925f3e317a3f30835cfd6/google/rpc/status.proto)
message.

#### operator std::string\_view

|||
----------------------------------- | ---
`operator std::string_view() const` | (1)

Returns a `std::string_view` representation of the status.

#### code

|||
------------------------------ | ---
`code_t code() const noexcept` | (1)

Returns the status code.

#### str

|||
------------------------------ | ---
`std::string_view str() const` | (1)

Returns a `std::string_view` representation of the status.

### Example

```cpp
grpcxx::status s1; // ok
grpcxx::status s2(grpcxx::status::code_t::internal); // internal
```
