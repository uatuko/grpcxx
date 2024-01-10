# Hello, World! 👋

This is the obligatory ["Hello, World!"](https://en.wikipedia.org/wiki/%22Hello,_World!%22_program) example to illustrate
the basics (which  is also used to benchmarks agaist the [official gRPC implementations](https://github.com/grpc/grpc/tree/master/examples/cpp/helloworld)).


## Walkthrough

### `greeter.proto`

[`proto/helloworld/v1/greeter.proto`](https://github.com/uatuko/grpcxx/blob/main/examples/helloworld/proto/helloworld/v1/greeter.proto)
defines a `Greeter` gRPC service with one method. This is a slightly changed version of
[`helloworld.proto`](https://github.com/grpc/grpc/blob/master/examples/protos/helloworld.proto) from the official examples.

`.proto` files are compiled using a CMake custom build rule [here](https://github.com/uatuko/grpcxx/blob/821a3bfe3f91eb523f515e56b9482adf4d66011a/examples/helloworld/proto/CMakeLists.txt#L19).

> 💡 You can find more info on compiling `.proto` files [here](../../docs/protoc-gen-grpcxx.md).

### `main.cpp`

All the code necessary to implement a fully working gRPC server based on the _Greeter_ service definitions are in
[`main.cpp`](https://github.com/uatuko/grpcxx/blob/main/examples/helloworld/main.cpp).

There are two examples of how to implement the _Greeter_ service.

#### Example 1

[First example](https://github.com/uatuko/grpcxx/blob/821a3bfe3f91eb523f515e56b9482adf4d66011a/examples/helloworld/main.cpp#L9)
implements the RPC logic by specialising the template `call()` method in `ServiceImpl` struct generated when compiling `.proto` files.

The generated `ServiceImpl` would look like this;
```cpp
struct ServiceImpl {
	template <typename T>
	typename T::result_type call(grpcxx::context &, const typename T::request_type &) {
		return {grpcxx::status::code_t::unimplemented, std::nullopt};
	}
};
```

Here's an extract of the RPC implementation to send back a response.
```cpp
template <>
rpcHello::result_type ServiceImpl::call<rpcHello>(
	grpcxx::context &, const GreeterHelloRequest &req) {
	GreeterHelloResponse res;
	res.set_message("Hello `" + req.name() + "` 👋");
	return {grpcxx::status::code_t::ok, res};
}
```

#### Example 2

The [second example](https://github.com/uatuko/grpcxx/blob/821a3bfe3f91eb523f515e56b9482adf4d66011a/examples/helloworld/main.cpp#L18)
demonstrates how you could define your own class and implement the RPC logic.

The only requirement to define your own implementation class is to have the template `call()` member function.

> 🎓 The reason it is required to have a template member function (instead of a member function) is to make it possible
to implement multiple gRPC methods with the same request (and response) type.

### Build & Run

You can simply run `make examples` (at the project root) to build the examples. This will produce an executable at
`.build/bin/grpcxx-examples_helloworld`.

```
❯ make examples
cmake --build .build --target examples
```

```
❯ ./.build/bin/grpcxx-examples_helloworld
Listening on [127.0.0.1:7000] ...
```

### Send requests

#### grpcurl

[grpcurl](https://github.com/fullstorydev/grpcurl) is a handy little tool to send gRPC requests without having to
implement a client.

Here's an example using `grpcurl` to send a request.
```
❯ grpcurl -vv \
  -proto examples/helloworld/proto/helloworld/v1/greeter.proto \
  -plaintext \
  -d '{"name": "World"}' \
  localhost:7000 helloworld.v1.Greeter/Hello

Resolved method descriptor:
rpc Hello ( .helloworld.v1.GreeterHelloRequest ) returns ( .helloworld.v1.GreeterHelloResponse );

Request metadata to send:
(empty)

Response headers received:
content-type: application/grpc

Estimated response size: 20 bytes

Response contents:
{
  "message": "Hello `World` 👋"
}

Response trailers received:
(empty)
Sent 1 request and received 1 response
```

#### nghttp

gRPC operates on top of HTTP/2. This allows us to use HTTP/2 clients like [nghttp](https://nghttp2.org/documentation/nghttp.1.html)
to send requests. But we will need to construct a binary payload with the gRPC message so the server can understand the
gRPC request.

> 💡 You can construct a gRPC message by serialising the protobuf message and appending the 5 byte prefix (see the
[gRPC over HTTP/2](https://github.com/grpc/grpc/blob/master/doc/PROTOCOL-HTTP2.md) spec for more info). For convenience
you could use [hello.grpc-lpm.data](https://github.com/uatuko/grpcxx/blob/main/examples/helloworld/testdata/hello.grpc-lpm.data)
which contains a valid binary payload.

```
❯ nghttp --verbose \
  --header='content-type: application/grpc' \
  --data=examples/helloworld/testdata/hello.grpc-lpm.data \
  http://localhost:7000/helloworld.v1.Greeter/Hello
[  0.002] Connected
[  0.003] send SETTINGS frame <length=12, flags=0x00, stream_id=0>
          (niv=2)
          [SETTINGS_MAX_CONCURRENT_STREAMS(0x03):100]
          [SETTINGS_INITIAL_WINDOW_SIZE(0x04):65535]
[  0.003] send PRIORITY frame <length=5, flags=0x00, stream_id=3>
          (dep_stream_id=0, weight=201, exclusive=0)
[  0.003] send PRIORITY frame <length=5, flags=0x00, stream_id=5>
          (dep_stream_id=0, weight=101, exclusive=0)
[  0.003] send PRIORITY frame <length=5, flags=0x00, stream_id=7>
          (dep_stream_id=0, weight=1, exclusive=0)
[  0.003] send PRIORITY frame <length=5, flags=0x00, stream_id=9>
          (dep_stream_id=7, weight=1, exclusive=0)
[  0.003] send PRIORITY frame <length=5, flags=0x00, stream_id=11>
          (dep_stream_id=3, weight=1, exclusive=0)
[  0.003] send HEADERS frame <length=78, flags=0x24, stream_id=13>
          ; END_HEADERS | PRIORITY
          (padlen=0, dep_stream_id=11, weight=16, exclusive=0)
          ; Open new stream
          :method: POST
          :path: /helloworld.v1.Greeter/Hello
          :scheme: http
          :authority: localhost:7000
          accept: */*
          accept-encoding: gzip, deflate
          user-agent: nghttp2/1.56.0
          content-length: 18
          content-type: application/grpc
[  0.003] send DATA frame <length=18, flags=0x01, stream_id=13>
          ; END_STREAM
[  0.006] recv SETTINGS frame <length=6, flags=0x00, stream_id=0>
          (niv=1)
          [SETTINGS_MAX_CONCURRENT_STREAMS(0x03):10]
[  0.006] recv SETTINGS frame <length=0, flags=0x01, stream_id=0>
          ; ACK
          (niv=0)
[  0.006] recv (stream_id=13) :status: 200
[  0.006] recv (stream_id=13) content-type: application/grpc
[  0.006] recv HEADERS frame <length=14, flags=0x04, stream_id=13>
          ; END_HEADERS
          (padlen=0)
          ; First response header

Hello `grpc-client` 👋[  0.006] recv DATA frame <length=31, flags=0x00, stream_id=13>
[  0.006] recv (stream_id=13) grpc-status: 0
[  0.006] recv HEADERS frame <length=12, flags=0x05, stream_id=13>
          ; END_STREAM | END_HEADERS
          (padlen=0)
[  0.006] send GOAWAY frame <length=8, flags=0x00, stream_id=0>
          (last_stream_id=0, error_code=NO_ERROR(0x00), opaque_data(0)=[])
```

#### curl

Similar to `nghttp`, we could also use `curl` to send requests.

```
❯ curl --verbose \
  --http2-prior-knowledge \
  --header 'Content-Type: application/grpc' \
  --data-binary @examples/helloworld/testdata/hello.grpc-lpm.data \
  localhost:7000/helloworld.v1.Greeter/Hello
* Host localhost:7000 was resolved.
* IPv6: ::1
* IPv4: 127.0.0.1
*   Trying [::1]:7000...
* connect to ::1 port 7000 from ::1 port 65492 failed: Connection refused
*   Trying 127.0.0.1:7000...
* Connected to localhost (127.0.0.1) port 7000
* [HTTP/2] [1] OPENED stream for http://localhost:7000/helloworld.v1.Greeter/Hello
* [HTTP/2] [1] [:method: POST]
* [HTTP/2] [1] [:scheme: http]
* [HTTP/2] [1] [:authority: localhost:7000]
* [HTTP/2] [1] [:path: /helloworld.v1.Greeter/Hello]
* [HTTP/2] [1] [user-agent: curl/8.5.0]
* [HTTP/2] [1] [accept: */*]
* [HTTP/2] [1] [content-type: application/grpc]
* [HTTP/2] [1] [content-length: 18]
> POST /helloworld.v1.Greeter/Hello HTTP/2
> Host: localhost:7000
> User-Agent: curl/8.5.0
> Accept: */*
> Content-Type: application/grpc
> Content-Length: 18
>
< HTTP/2 200
< content-type: application/grpc
<
Warning: Binary output can mess up your terminal. Use "--output -" to tell
Warning: curl to output it to your terminal anyway, or consider "--output
Warning: <FILE>" to save to a file.
* Failure writing output to destination
* Connection #0 to host localhost left intact
```
