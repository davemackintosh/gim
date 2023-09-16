.PHONY: clean
clean:
	xmake clean

.PHONY: compile_commands.json
compile_commands.json:
	xmake project --yes -k compile_commands

.PHONY: debug
debug: compile_commands.json
	xmake f -m debug
	xmake build -D gim

.PHONY: build
build:
	cmake --debug-output -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
	cmake --build build

.PHONY: run
run: build
	xmake build --yes gim
	xmake run gim

.PHONY: test
test: compile_commands.json
	xmake build -v test_gim
	xmake run test_gim
