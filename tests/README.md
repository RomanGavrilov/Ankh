# Integration Tests

This directory contains integration tests for the Ankh Vulkan renderer.

## Quick Start (Automated)

The easiest way to build and run tests is using the automated test runner:

```bash
pip install pytest
python tests/run_tests.py
```

This script automatically:
1. Configures CMake if needed
2. Builds the Ankh executable
3. Runs pytest with the correct settings

### Options

```bash
# Use Release build
python tests/run_tests.py --build-type Release

# Skip build step (use existing build)
python tests/run_tests.py --skip-build

# Custom build directory
python tests/run_tests.py --build-dir path/to/build
```

## Manual Testing

If you prefer to build and test separately:

```bash
# Build first
mkdir -p build && cd build
cmake ..
cmake --build . --target Ankh

# Then run tests
pytest tests/test_integration.py -v
```

The test automatically finds the Ankh executable in common build locations.

### With explicit executable path

```bash
pytest tests/test_integration.py -v --ankh-path=path/to/Ankh.exe
```

Or using environment variable:

```bash
set ANKH_EXECUTABLE=path\to\Ankh.exe   # Windows
export ANKH_EXECUTABLE=path/to/Ankh    # Linux/macOS
pytest tests/test_integration.py -v
```

## Test Approach

The integration tests run the Ankh application as a **separate process**, completely isolated from the test framework. This approach:

- Does not modify any production code
- Tests the actual released binary
- Captures validation errors from stderr
- Detects crashes via process exit codes

## Prerequisites

### System Dependencies

- Python 3.8+
- pytest (`pip install pytest`)
- Vulkan SDK and validation layers
- Display (X11 on Linux, or use Xvfb for headless)
- `glslc` shader compiler (for building Ankh)

### On Windows

1. Install Python 3.8+ from https://www.python.org/downloads/
2. Install the Vulkan SDK from https://vulkan.lunarg.com/sdk/home
3. Build Ankh with CMake/Visual Studio

### On Ubuntu/Debian

```bash
sudo apt-get install -y python3 python3-pip vulkan-validationlayers mesa-vulkan-drivers xvfb glslc
pip install pytest
```

## Building the App

Build the Ankh application first:

### Windows (Visual Studio)

```cmd
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON
cmake --build . --config Release
```

### Linux/macOS

```bash
mkdir -p build && cd build
cmake .. -DBUILD_TESTS=ON
make
```

## Running Tests

### Using pytest (Recommended)

```bash
# From repo root
pytest tests/test_integration.py -v

# Or with explicit path
pytest tests/test_integration.py -v --ankh-path=build/src/Release/Ankh.exe
```

### Using CTest

```bash
cd build
ctest --output-on-failure
```

### Headless (CI/Server - Linux)

For headless environments, use Xvfb with the lavapipe software Vulkan driver:

```bash
# Start Xvfb
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99

# Use software Vulkan driver (lavapipe)
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.x86_64.json

# Run tests
pytest tests/test_integration.py -v
```

## Test Cases

### `test_starts_without_crash`

Runs the application for a few seconds and verifies it doesn't crash (exit code >= 0).

### `test_starts_without_validation_errors`

Runs the application and captures stderr output to check for Vulkan validation layer error messages.

### `test_content_is_rendered`

Verifies that the application runs for the expected duration (doesn't exit immediately), indicating it's actually rendering content.
