#pragma once

#ifndef GRPCXX_USE_ASIO
#include "uv/server.h"
#else
#include "asio/server.h"
#endif

namespace grpcxx {
#ifndef GRPCXX_USE_ASIO
using server = uv::detail::server;
#else
using server = asio::server;
#endif
} // namespace grpcxx
