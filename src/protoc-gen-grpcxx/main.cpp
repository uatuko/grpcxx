#include <google/protobuf/compiler/plugin.h>

#include "grpcxx.h"

int main(int argc, char *argv[]) {
	Grpcxx g;
	return google::protobuf::compiler::PluginMain(argc, argv, &g);
}
