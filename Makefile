.PHONY: build
build:
	xmake project -k compile_commands
	xmake build

.PHONY: run
run: build
	xmake run

.PHONY: test
test:
	xmake build test_gim
	xmake run test_gim
