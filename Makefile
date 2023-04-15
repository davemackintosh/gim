.PHONY: clean
clean:
	xmake clean

.PHONY: compile_commands.json
compile_commands.json:
	@xmake project -k compile_commands --yes

.PHONY: debug
debug: compile_commands.json
	@xmake build gim

.PHONY: build
build: compile_commands.json
	xmake build gim

.PHONY: run
run: build
	xmake run

.PHONY: test
test: compile_commands.json
	xmake build -v test_gim
	xmake run test_gim
