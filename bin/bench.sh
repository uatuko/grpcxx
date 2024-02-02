#!/bin/sh

# Ensure the binary is up-to-date
make

# Run the server and send it to the background
./.build/bin/grpcxx-examples_helloworld &
pid=$!
echo "[init] server pid: ${pid}"

# Trap ctrl+c and cleanup
trap "kill ${pid}; exit 1" INT
sleep 1

echo "[bench] warming up"
nghttp --stat --null-out --multiply=10 \
	--header='content-type: application/grpc' \
	--data=examples/helloworld/testdata/hello.grpc-lpm.data \
	http://localhost:7000/helloworld.v1.Greeter/Hello

sleep 1
echo "[bench] start"
duration=3

: 1a
echo "[bench] :begin - 1a"
h2load --clients=1 --duration=${duration} \
	--header='Content-Type: application/grpc' \
	--data=examples/helloworld/testdata/hello.grpc-lpm.data \
	http://localhost:7000/helloworld.v1.Greeter/Hello
echo "[bench] :end - 1a"

: 1b
echo "[bench] :begin - 1b"
h2load --clients=1 --max-concurrent-streams=10 --duration=${duration} \
	--header='Content-Type: application/grpc' \
	--data=examples/helloworld/testdata/hello.grpc-lpm.data \
	http://localhost:7000/helloworld.v1.Greeter/Hello
echo "[bench] :end - 1b"

: 2a
echo "[bench] :begin - 2a"
h2load --clients=10 --duration=${duration} \
	--header='Content-Type: application/grpc' \
	--data=examples/helloworld/testdata/hello.grpc-lpm.data \
	http://localhost:7000/helloworld.v1.Greeter/Hello
echo "[bench] :end - 2a"

: 2b
echo "[bench] :begin - 2b"
h2load --clients=10 --max-concurrent-streams=10 --duration=${duration} \
	--header='Content-Type: application/grpc' \
	--data=examples/helloworld/testdata/hello.grpc-lpm.data \
	http://localhost:7000/helloworld.v1.Greeter/Hello
echo "[bench] :end - 2b"

: 3a
echo "[bench] :begin - 3a"
h2load --clients=100 --duration=${duration} \
	--header='Content-Type: application/grpc' \
	--data=examples/helloworld/testdata/hello.grpc-lpm.data \
	http://localhost:7000/helloworld.v1.Greeter/Hello
echo "[bench] :end - 3a"

: 3b
echo "[bench] :begin - 3b"
h2load --clients=100 --max-concurrent-streams=10 --duration=${duration} \
	--header='Content-Type: application/grpc' \
	--data=examples/helloworld/testdata/hello.grpc-lpm.data \
	http://localhost:7000/helloworld.v1.Greeter/Hello
echo "[bench] :end - 3b"

sleep 1
echo "[bench] done"
kill ${pid}
