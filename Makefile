CMAKE_CMD=cmake --debug-output -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake

.PHONY: clean
clean:
	cmake clean

.PHONY: configure
configure:
	${CMAKE_CMD}

.PHONY: build
build: configure
	mkdir build || true
	${CMAKE_CMD} --build build

.PHONY: run
run: build
	cd build; ./arteeyes

build-windows:
	${CMAKE_CMD} -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=cmake/windows-toolchain.cmake
