# Integration Tests

This directory contains integration tests for the Ankh Vulkan renderer.

## Quick Start (One Command)

### Windows

After building Ankh, run from the `tests` directory:

```cmd
run_tests.bat
```

Or with a specific executable path:

```cmd
run_tests.bat C:\path\to\Ankh.exe
```

### Linux/macOS

```bash
python3 run_tests.py
```

Or with a specific executable path:

```bash
python3 run_tests.py /path/to/Ankh
```

The scripts automatically:
- Create a Python virtual environment
- Install pytest
- Locate the Ankh executable
- Run all tests

## Test Approach

The integration tests run the Ankh application as a **separate process**, completely isolated from the test framework. This approach:

- Does not modify any production code
- Tests the actual released binary
- Captures validation errors from stderr
- Detects crashes via process exit codes

## Prerequisites

### System Dependencies

- Python 3.8+
- Vulkan SDK and validation layers
- Display (X11 on Linux, or use Xvfb for headless)
- `glslc` shader compiler (for building Ankh)

### On Windows

1. Install Python 3.8+ from https://www.python.org/downloads/
2. Install the Vulkan SDK from https://vulkan.lunarg.com/sdk/home
3. Build Ankh with CMake/Visual Studio

### On Ubuntu/Debian

```bash
sudo apt-get install -y python3 python3-pip python3-venv vulkan-validationlayers mesa-vulkan-drivers xvfb glslc
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

### Using the Test Runner Scripts (Recommended)

The test runner scripts handle everything automatically:

**Windows:**
```cmd
cd tests
run_tests.bat
```

**Linux/macOS:**
```bash
cd tests
python3 run_tests.py
```

### Using CTest

```bash
cd build
ctest --output-on-failure
```

### Using pytest directly

```bash
cd build/src
python -m pytest ../../tests/test_integration.py -v
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
cd tests
python3 run_tests.py
```

## Test Cases

### `test_starts_without_crash`

Runs the application for a few seconds and verifies it doesn't crash (exit code >= 0).

### `test_starts_without_validation_errors`

Runs the application and captures stderr output to check for Vulkan validation layer error messages.

### `test_content_is_rendered`

Verifies that the application runs for the expected duration (doesn't exit immediately), indicating it's actually rendering content.
