.PHONY: clean
clean:
	xmake clean

.PHONY: compile_commands.json
compile_commands.json:
	xmake f -m debug
	xmake project -k compile_commands

.PHONY: debug
debug:
	xmake f -m debug
	xmake build gim

.PHONY: build
build: compile_commands.json
	xmake build gim

.PHONY: run
run: build
	xmake run

.PHONY: test
test:
	xmake f -m debug --yes
	xmake build test_gim
	xmake run test_gim
