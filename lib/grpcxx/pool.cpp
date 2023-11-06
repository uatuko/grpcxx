#include "pool.h"

namespace grpcxx {
namespace detail {
pool::pool(std::size_t n) : _idx(0), _threads(), _workers(n) {
	if (n == 0) {
		n        = 1;
		_workers = workers_t(n);
	}

	for (std::size_t i = 0; i < n; i++) {
		_threads.emplace_front(&worker::run, std::ref(_workers[i]));
	}
}

pool::awaiter pool::schedule() noexcept {
	return awaiter(this);
}

worker &pool::worker() noexcept {
	_idx = (_idx + 1) % _workers.size();
	return _workers[_idx];
}
} // namespace detail
} // namespace grpcxx
