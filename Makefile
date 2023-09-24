.PHONY: clean
clean:
	cmake clean

.PHONY: build
build:
	cmake --debug-output -G Ninja -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_CXX_FLAGS="-fdiagnostics-color=always"
	cmake --build build 

