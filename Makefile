VCPKG = vcpkg
VCPKG_PKGS = nlohmann-json nanoflann
VCPKG_TOOLCHAIN = $(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake
BUILD_DIR = build

.PHONY: all vcpkg clean debug release help build test

all: vcpkg
	cmake -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S .
	cmake --build $(BUILD_DIR) --config Release

vcpkg:
# 	$(VCPKG) install $(VCPKG_PKGS)
	$(VCPKG) install

build:
	cmake --build $(BUILD_DIR) --config Release

release:
	cmake -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S .
	cmake --build $(BUILD_DIR)

debug:
	cmake -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_TOOLCHAIN) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S .
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Makefile commands:"
	@echo "  all      - Build the project in Release mode (default)"
	@echo "  vcpkg    - Install required packages using vcpkg"
	@echo "  release  - Build the project in Release mode"
	@echo "  debug    - Build the project in Debug mode"
	@echo "  clean    - Remove the build directory"
	@echo "  help     - Show this help message"

test:
	./test.sh
