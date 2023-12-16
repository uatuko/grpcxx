#pragma once

namespace grpcxx {
class context {
public:
	context()                = default;
	context(context &&)      = default;
	context(const context &) = delete;

private:
};
} // namespace grpcxx
