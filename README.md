# Goethe Dialog System

A modern C++ library for visual novel dialog management with YAML support and compression capabilities.

## Overview

Goethe Dialog System is a C/C++ library that provides functionality for loading, parsing, and manipulating dialog data in YAML format. It's designed specifically for visual novel and interactive storytelling applications, featuring a flexible compression system with multiple backend implementations.

## Features

- **YAML-based dialog format**: Load and save dialog data in structured YAML format
- **C and C++ APIs**: Use from both C and C++ applications
- **Character dialog management**: Support for character names, expressions, moods, and timing
- **Compression system**: Multiple compression backends with automatic selection
- **Cross-platform**: Works on Linux, Windows, and macOS
- **Header-only dependencies**: Only requires yaml-cpp

## Project Structure

```
goethe/
├── src/                    # Source code
│   ├── engine/            # Core engine components
│   │   ├── core/          # Core dialog system
│   │   │   ├── compression/  # Compression backends
│   │   │   │   └── implementations/
│   │   │   │       ├── null.cpp    # No-op compression
│   │   │   │       └── zstd.cpp    # Zstd compression
│   │   │   └── dialog.cpp          # Dialog implementation
│   │   └── util/          # Utility functions
│   ├── tools/             # Command-line tools
│   └── tests/             # Test files
├── include/               # Public headers
│   └── goethe/           # Goethe library headers
│       ├── backend.hpp           # Compression backend interface
│       ├── factory.hpp           # Compression factory
│       ├── manager.hpp           # High-level compression manager
│       ├── dialog.hpp            # Dialog system interface
│       ├── goethe_dialog.h       # C API
│       ├── null.hpp              # Null compression backend
│       └── zstd.hpp              # Zstd compression backend
├── build/                 # Build artifacts (generated)
├── scripts/               # Build and utility scripts
├── schemas/               # Schema definitions
├── docs/                  # Documentation
├── third_party/           # Third-party dependencies
└── CMakeLists.txt         # CMake configuration
```

## Dependencies

### Required
- CMake 3.20+
- C++20 compatible compiler (Clang/GCC/MSVC)
- yaml-cpp

### Optional
- zstd (for compression)

## Building

### Quick Start

```bash
# Clone the repository
git clone <repository-url>
cd goethe

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run tests
./simple_test
```

### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# Build
make -j$(nproc)

# Install (optional)
sudo make install
```

## Usage

### C++ API

```cpp
#include <goethe/dialog.hpp>
#include <goethe/manager.hpp>
#include <fstream>

// Initialize compression manager
auto& comp_manager = goethe::CompressionManager::instance();
comp_manager.initialize("zstd"); // or auto-select

// Load dialog from file
std::ifstream file("dialog.yaml");
goethe::Dialogue dialog = goethe::read_dialog(file);

// Access dialog properties
std::cout << "Title: " << dialog.title << std::endl;
std::cout << "Lines: " << dialog.lines.size() << std::endl;

// Iterate through dialog lines
for (const auto& line : dialog.lines) {
    std::cout << line.character << ": " << line.phrase << std::endl;
}

// Compress data
std::vector<uint8_t> data = { /* your data */ };
auto compressed = comp_manager.compress(data);
auto decompressed = comp_manager.decompress(compressed);
```

### C API

```c
#include <goethe/goethe_dialog.h>

// Create dialog object
GoetheDialog* dialog = goethe_dialog_create();

// Load from YAML file
if (goethe_dialog_load_from_file(dialog, "dialog.yaml") == 0) {
    // Get dialog info
    printf("Title: %s\n", goethe_dialog_get_title(dialog));
    printf("Lines: %d\n", goethe_dialog_get_line_count(dialog));
    
    // Get specific line
    GoetheDialogLine* line = goethe_dialog_get_line(dialog, 0);
    if (line) {
        printf("%s: %s\n", line->character, line->phrase);
    }
}

// Clean up
goethe_dialog_destroy(dialog);
```

## Dialog YAML Format

```yaml
dialogue_id: chapter1_intro
title: Chapter 1: The Beginning
mode: visual_novel
default_time: 3.0
lines:
  - character: Alice
    phrase: Hello, welcome to our story!
    direction: center
    expression: happy
    mood: friendly
    time: 2.5
  - character: Bob
    phrase: Thank you, I'm excited to begin!
    direction: left
    expression: excited
    mood: enthusiastic
    time: 3.0
```

## Compression System

The compression system supports multiple backends with automatic selection:

### Available Backends

1. **Zstd** (recommended): Best compression ratio and speed
2. **Null**: No compression (for testing/fallback)

### Usage Examples

```cpp
// High-level usage
auto& manager = goethe::CompressionManager::instance();
manager.initialize("zstd"); // or auto-select
auto compressed = manager.compress(data);
auto decompressed = manager.decompress(compressed);

// Direct backend usage
auto backend = goethe::create_compression_backend("zstd");
backend->set_compression_level(10);
auto compressed = backend->compress(data);

// Global convenience functions
auto compressed = goethe::compress_data(data.data(), data.size(), "zstd");
```

## Testing

Run the test suite:

```bash
# Build tests
cd build
make

# Run tests
./simple_test
```

## Development

### Code Style

- Follow the existing code style
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions small and focused

### Adding New Features

1. Add source files to `src/`
2. Add headers to `include/goethe/`
3. Update `CMakeLists.txt` with new files
4. Add tests in `src/tests/`
5. Update documentation

### Project Organization

- **Source Code**: All `.cpp` files go in `src/`
- **Headers**: Public headers go in `include/goethe/`
- **Tests**: Test files go in `src/tests/`
- **Tools**: Command-line tools go in `src/tools/`
- **Scripts**: Build and utility scripts go in `scripts/`

## Architecture

### Compression System

The compression system uses the **Strategy Pattern** combined with a **Factory Pattern**:

- **Strategy Pattern**: Each compression algorithm implements the `CompressionBackend` interface
- **Factory Pattern**: `CompressionFactory` creates backends by name or auto-selects the best available
- **Manager Pattern**: `CompressionManager` provides a high-level, easy-to-use API
- **Automatic Registration**: Backends are automatically registered and available
- **Priority-based Selection**: Zstd → Null (best to fallback)

### Benefits

- **Extensibility**: Easy to add new compression algorithms
- **Flexibility**: Can switch backends at runtime
- **Maintainability**: Clean separation of concerns
- **Performance**: Optimized for each algorithm
- **Reliability**: Graceful fallbacks and error handling
- **Usability**: Multiple levels of abstraction

## License

This project is open source. See LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## Roadmap

- [ ] Add LZ4 compression backend
- [ ] Add Zlib compression backend
- [ ] Implement package system
- [ ] Add encryption support
- [ ] Create GUI tools
- [ ] Add more dialog formats
