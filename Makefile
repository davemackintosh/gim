.PHONY: clean
clean:
	cmake clean

.PHONY: configure
configure:
	cmake --debug-output -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

.PHONY: build
build: configure
	cmake --build build 

