.PHONY: clean
clean:
	xmake clean

.PHONY: compile_commands.json
compile_commands.json:
	@xmake project -k compile_commands

.PHONY: debug
debug: compile_commands.json
	xmake f -m debug
	xmake build gim

.PHONY: build
build:
	xmake build gim

.PHONY: run
run: build
	xmake build --yes gim
	xmake run gim

.PHONY: test
test: compile_commands.json
	xmake build -v test_gim
	xmake run test_gim
