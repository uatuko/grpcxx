builddir  = .build
bindir    = $(builddir)/bin
buildfile = $(builddir)/build.ninja

examplesdir    = examples
experimentsdir = .experiments
libdir         = lib
srcdir         = src

sources := $(shell find $(examplesdir) -type f -name '*.h' -o -name '*.cpp' -o -name '*.proto')
sources += $(shell find $(experimentsdir) -type f -name '*.h' -o -name '*.cpp' -o -name '*.proto')
sources += $(shell find $(libdir) -type f -name '*.h' -o -name '*.cpp')
sources += $(shell find $(srcdir) -type f -name '*.h' -o -name '*.cpp')

.PHONY: all clean examples lint lint\:ci lint\:fix
.SILENT: lint lint\:fix

all: $(buildfile)
	cmake --build $(builddir) --target all

$(buildfile):
	cmake -B $(builddir) -G Ninja

clean:
	cmake --build $(builddir) --target clean

examples: $(buildfile)
	cmake --build $(builddir) --target examples

experiments: $(buildfile)
	cmake --build $(builddir) --target experiments

lint:
ifeq (, $(shell which clang-format))
	echo '\033[1;41m WARN \033[0m clang-format not found, not linting files';
else
	clang-format --style=file --dry-run $(sources)
endif

lint\:ci:
	clang-format --style=file --dry-run --Werror $(sources)

lint\:fix:
	clang-format --style=file -i $(sources)
