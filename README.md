## Goethe Engine (skeleton)

Minimal scaffold for the Goethe Engine described in the architectural overview. It builds a library exposing a C ABI (`sdk/goethe.h`), includes tiny sample apps, and a basic GoogleTest suite.

### Build (quick start)

```
cmake -S . -B build
cmake --build build -j
```

### Samples

- hello_vn (console only):
  ```
  cmake --build build --target hello_vn
  ./build/samples/hello_vn/hello_vn
  ```

- visual_vn (requires SDL3):
  - **Recommended (vendored SDL3)** - fetches and builds SDL3 automatically:
    ```
    cmake -S . -B build -DGOETHE_BACKEND_SDL3=ON -DGOETHE_VENDOR_SDL3=ON
    cmake --build build --target visual_vn
    ./build/samples/visual_vn/visual_vn
    ```
  - **Alternative (system SDL3)** - requires SDL3 development package:
    ```
    # Install SDL3 dev package first (e.g., pacman -S sdl3)
    cmake -S . -B build -DGOETHE_BACKEND_SDL3=ON
    cmake --build build --target visual_vn
    ./build/samples/visual_vn/visual_vn
    ```
  - Note: `GOETHE_SDL3_HEADLESS=ON` will skip building `visual_vn` (for CI/headless builds).

### Tests (GoogleTest + CTest)

```
cmake -S . -B build -DGOETHE_BUILD_TESTS=ON
cmake --build build --target goethe_tests -j
ctest --test-dir build --output-on-failure
```

### Tools

Build the `goethec` CLI (disabled by default):

```
cmake -S . -B build -DGOETHE_BUILD_TOOLS=ON
cmake --build build --target goethec
./build/tools/goethec --help
```

### CMake options (defaults)

- **GOETHE_BUILD_SHARED [ON]**: Build the engine as a shared library; otherwise static.
- **GOETHE_BUILD_TOOLS [OFF]**: Build CLI tools (e.g., `goethec`).
- **GOETHE_BUILD_TESTS [OFF]**: Enable GoogleTest and the test suite.
- **GOETHE_BACKEND_SDL3 [OFF]**: Enable SDL3 rendering backend integration.
- **GOETHE_VENDOR_SDL3 [OFF]**: Fetch/build SDL3 with the project (when needed).
- **GOETHE_INSTALL_SDL3 [OFF]**: Include SDL3 in install rules (implies vendoring).
- **GOETHE_SDL3_HEADLESS [OFF]**: Build SDL3 for headless/offscreen only.
- **GOETHE_BACKEND_CPU [OFF]**: Placeholder toggle for a CPU raster backend.

### Install and package config

Install the library, headers, and schemas; and generate a CMake package:

```
cmake --install build
```

Downstream usage:

```
find_package(Goethe REQUIRED)
target_link_libraries(your_app PRIVATE Goethe::goethe)
```

### Notes and status

- C++20, hidden visibility by default; exceptions and RTTI disabled for the engine library.
- Engine is a minimal stub: accepts renderer names ("cpu", "sdl", "sdl_software"), exposes basic capabilities, and runs a no-op tick.
- Tests cover engine creation/destruction, renderer selection, and capability invariants.

### Resources

- Visual novel overview and best practices: [How to Make Visual Novels](https://arimiadev.com/how-to-make-visual-novels/)
