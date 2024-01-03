# Errors

This example demonstrates how to use [`google.rpc.Status`](https://github.com/googleapis/googleapis/blob/75c44112205d44183c3419d2c9cf4224e3c81d90/google/rpc/status.proto)
messages to send back detailed gRPC error responses. Refer to https://github.com/uatuko/grpcxx/issues/11 for additional
information on how this works.

> 💡 The protobuf files in `proto/google/**` are copied from [googleapis](https://github.com/googleapis/googleapis/tree/master/google).

e.g.
```
❯ grpcurl -vv -proto examples/errors/proto/examples/v1/errors.proto -plaintext localhost:7000 examples.v1.Errors/Test

Resolved method descriptor:
rpc Test ( .google.protobuf.Empty ) returns ( .google.protobuf.Empty );

Request metadata to send:
(empty)

Response headers received:
content-type: application/grpc

Response trailers received:
(empty)
Sent 0 requests and received 0 responses
ERROR:
  Code: InvalidArgument
  Message: Sample error message

```
