# Default build type
BUILD_TYPE ?= Release
BUILD_DIR = build

.PHONY: all clean test debug release

all: release

# Create build directory and generate build files
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Debug build
debug: $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug ..
	cmake --build $(BUILD_DIR)

# Release build
release: $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release ..
	cmake --build $(BUILD_DIR)

# Run tests
test: $(BUILD_DIR)
	cd $(BUILD_DIR) && ctest --output-on-failure

# Clean build directory
clean:
	rm -rf $(BUILD_DIR)
