# Make
MAKEFLAGS += --no-print-directory

# Compiler
CC	:= clang
CXX	:= clang++

# Files
PROJECT	:= $(shell grep -m 1 "^project" CMakeLists.txt | cut -c9- | cut -d" " -f1)
SOURCES	:= $(shell find src -type f)

# Targets
all: build/llvm/CMakeCache.txt $(SOURCES)
	@cmake --build build/llvm

build/llvm:
	@mkdir -p build/llvm

debug: build/llvm CMakeLists.txt
	@cd build/llvm && CC="$(CC)" CXX="$(CXX)" cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX:PATH=../.. ../..

release: build/llvm CMakeLists.txt
	@cd build/llvm && CC="$(CC)" CXX="$(CXX)" cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=../.. ../..

clean:
	@rm -rf build/llvm

install: release
	@cmake --build build/llvm --target install

pack: install
	tar czf $(PROJECT).tar.gz --transform 's/^bin/$(PROJECT)/' bin

run: all
	build/llvm/$(PROJECT)
