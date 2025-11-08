.PHONY: build clean

BUILD_DIR := build

build: configure
	cmake --build $(BUILD_DIR)

configure:
	cmake -B $(BUILD_DIR) -G Ninja

clean:
	rm -rf $(BUILD_DIR)
