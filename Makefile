.PHONY: build
build:
	mkdir -p build
	cmake -E chdir build cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
	cmake --build build

.PHONY: test
test:
	cd build && ctest --output-on-failure
