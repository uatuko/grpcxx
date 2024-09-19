#include "server.h"

#include <cstdio>

int main() {
	server s;

	try {
		s.run("127.0.0.1", 7000);
	} catch (const std::exception &e) {
		std::fprintf(stderr, "[fatal] error: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
