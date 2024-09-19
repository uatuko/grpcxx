#include "grpcxx.h"

#include <google/protobuf/compiler/plugin.h>

int main(int argc, char *argv[]) {
	Grpcxx g;
	return google::protobuf::compiler::PluginMain(argc, argv, &g);
}
