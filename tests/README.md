# Integration Tests

This directory contains integration tests for the Ankh Vulkan renderer.

## Test Approach

The integration tests run the Ankh application as a **separate process**, completely isolated from the test framework. This approach:

- Does not modify any production code
- Tests the actual released binary
- Captures validation errors from stderr
- Detects crashes via process signals

## Prerequisites

### System Dependencies

- GoogleTest library (`libgtest-dev` on Ubuntu/Debian)
- Vulkan SDK and validation layers
- X11 display (or Xvfb for headless testing)
- `glslc` shader compiler

### On Ubuntu/Debian

```bash
sudo apt-get install -y libgtest-dev vulkan-validationlayers mesa-vulkan-drivers xvfb glslc
```

## Building Tests

Tests are built by default with the project:

```bash
mkdir -p build && cd build
cmake .. -DBUILD_TESTS=ON
make
```

To disable tests:

```bash
cmake .. -DBUILD_TESTS=OFF
```

## Running Tests

### With Real Display

If you have a display available:

```bash
cd build
ctest --output-on-failure
```

Or run directly:

```bash
cd build/tests
./integration_test
```

### Headless (CI/Server)

For headless environments, use Xvfb with the lavapipe software Vulkan driver:

```bash
# Start Xvfb
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99

# Use software Vulkan driver (lavapipe)
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.x86_64.json

# Run tests
cd build
ctest --output-on-failure
```

## Test Cases

### `AppStartupTest.StartsWithoutCrash`

Runs the application for a few seconds and verifies it doesn't crash (no signals like SIGSEGV, SIGABRT, etc.).

### `AppStartupTest.StartsWithoutValidationErrors`

Runs the application and captures stderr output to check for Vulkan validation layer error messages.

### `AppStartupTest.ContentIsRendered`

Verifies that the application runs for the expected duration (doesn't exit immediately), indicating it's actually rendering content.
