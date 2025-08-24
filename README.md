# Goethe Dialog System

A modern C++ library for managing multiple-path dialog systems—including visual novels and other interactive narratives—with YAML support, compression capabilities, and performance monitoring.

## Overview

Goethe Dialog System is a comprehensive C/C++ library that provides functionality for loading, parsing, and manipulating dialog data in YAML format. It's designed specifically for visual novel and interactive storytelling applications, featuring a flexible compression system with multiple backend implementations, comprehensive statistics tracking, and advanced testing capabilities.

## Features

- **Dual YAML formats**: Support for both simple and advanced GOETHE dialog formats
- **C and C++ APIs**: Use from both C and C++ applications
- **Character dialog management**: Support for character names, expressions, moods, portraits, and voice
- **Conditional logic**: Advanced condition system with flags, variables, and quest states
- **Effect system**: Comprehensive effect system for game state changes
- **Compression system**: Multiple compression backends with automatic selection
- **Statistics tracking**: Real-time performance monitoring and analysis
- **Comprehensive testing**: Google Test integration with multiple test suites
- **Cross-platform**: Works on Linux, Windows, and macOS
- **Development tools**: Command-line tools for analysis and management

## Project Structure

```
goethe/
├── src/                    # Source code
│   ├── engine/            # Core engine components
│   │   ├── core/          # Core dialog system
│   │   │   ├── compression/  # Compression backends
│   │   │   │   ├── implementations/
│   │   │   │   │   ├── null.cpp    # No-op compression
│   │   │   │   │   └── zstd.cpp    # Zstd compression
│   │   │   │   ├── backend.cpp     # Base interface
│   │   │   │   ├── factory.cpp     # Factory implementation
│   │   │   │   ├── manager.cpp     # Manager implementation
│   │   │   │   └── register_backends.cpp  # Backend registration
│   │   │   ├── dialog.cpp          # Dialog implementation
│   │   │   └── statistics.cpp      # Statistics tracking system
│   │   └── util/          # Utility functions
│   ├── tools/             # Command-line tools
│   │   ├── gdkg_tool.cpp          # Package management tool
│   │   └── statistics_tool.cpp    # Statistics analysis tool
│   └── tests/             # Comprehensive test suite
│       ├── test_dialog.cpp        # Dialog system tests
│       ├── test_compression.cpp   # Compression system tests
│       ├── test_basic.cpp         # Basic functionality tests
│       ├── statistics_test.cpp    # Statistics system tests
│       ├── simple_test.cpp        # Simple integration test
│       └── minimal_*.cpp          # Minimal test cases
├── include/               # Public headers
│   └── goethe/           # Goethe library headers
│       ├── backend.hpp           # Compression backend interface
│       ├── factory.hpp           # Compression factory
│       ├── manager.hpp           # High-level compression manager
│       ├── dialog.hpp            # Dialog system interface
│       ├── goethe_dialog.h       # C API
│       ├── null.hpp              # Null compression backend
│       ├── register_backends.hpp # Backend registration
│       ├── statistics.hpp        # Statistics tracking interface
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
- C++20 compatible compiler (Clang preferred, GCC fallback)
- yaml-cpp

### Optional
- zstd (for compression)
- OpenSSL (for package encryption and signing)
- Google Test (for testing)

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
ctest --verbose
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
#include <goethe/statistics.hpp>
#include <fstream>

// Initialize compression manager
auto& comp_manager = goethe::CompressionManager::instance();
comp_manager.initialize("zstd"); // or auto-select

// Enable statistics tracking
auto& stats_manager = goethe::StatisticsManager::instance();
stats_manager.enable_statistics(true);

// Load dialog from file
std::ifstream file("dialog.yaml");
goethe::Dialogue dialogue = goethe::read_dialogue(file);

// Access dialog properties
std::cout << "ID: " << dialogue.id << std::endl;
std::cout << "Nodes: " << dialogue.nodes.size() << std::endl;

// Iterate through dialog nodes
for (const auto& node : dialogue.nodes) {
    if (node.speaker) {
        std::cout << *node.speaker << ": " << node.line.text << std::endl;
    }
}

// Compress data with statistics tracking
std::vector<uint8_t> data = { /* your data */ };
auto compressed = comp_manager.compress(data);
auto decompressed = comp_manager.decompress(compressed);

// Get performance statistics
auto stats = stats_manager.get_backend_stats("zstd");
std::cout << "Compression ratio: " << stats.average_compression_ratio() << std::endl;
std::cout << "Throughput: " << stats.average_compression_throughput_mbps() << " MB/s" << std::endl;
```

### C API

```c
#include <goethe/goethe_dialog.h>

// Create dialog object
GoetheDialog* dialog = goethe_dialog_create();

// Load from YAML file
if (goethe_dialog_load_from_file(dialog, "dialog.yaml") == 0) {
    // Get dialog info
    printf("ID: %s\n", goethe_dialog_get_id(dialog));
    printf("Nodes: %d\n", goethe_dialog_get_node_count(dialog));
    
    // Get specific node
    GoetheDialogNode* node = goethe_dialog_get_node(dialog, 0);
    if (node) {
        printf("Speaker: %s\n", node->speaker ? node->speaker : "Narrator");
        printf("Text: %s\n", node->line.text);
    }
}

// Clean up
goethe_dialog_destroy(dialog);
```

## Dialog YAML Formats

### Simple Format

```yaml
id: chapter1_intro
nodes:
  - id: greeting
    speaker: alice
    line:
      text: Hello, welcome to our story!
  - id: response
    speaker: bob
    line:
      text: Thank you, I'm excited to begin!
```

### Advanced GOETHE Format

```yaml
kind: dialogue
id: chapter1_intro
startNode: intro

nodes:
  - id: intro
    speaker: marshal
    line:
      text: dlg_test.intro.text
      portrait: { id: marshal, mood: neutral }
      voice: { clipId: vo_test_intro }
    choices:
      - id: accept
        text: dlg_test.intro.choice.accept
        to: agree
        effects:
          - type: SET_FLAG
            target: test_accepted
            value: true
      - id: refuse
        text: dlg_test.intro.choice.refuse
        to: farewell
```

## Compression System

The compression system supports multiple backends with automatic selection and performance monitoring:

### Available Backends

1. **Zstd** (recommended): Best compression ratio and speed
2. **Null**: No compression (for testing/fallback)

### Usage Examples

```cpp
// High-level usage with statistics
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

## Statistics System

The statistics system provides real-time performance monitoring:

```cpp
// Enable statistics tracking
auto& stats_manager = goethe::StatisticsManager::instance();
stats_manager.enable_statistics(true);

// Perform operations (automatically tracked)
auto compressed = manager.compress(data);

// Get performance metrics
auto stats = stats_manager.get_backend_stats("zstd");
std::cout << "Compression ratio: " << stats.average_compression_ratio() << std::endl;
std::cout << "Success rate: " << stats.success_rate() << std::endl;
std::cout << "Throughput: " << stats.average_compression_throughput_mbps() << " MB/s" << std::endl;
```

## Testing

Run the comprehensive test suite:

```bash
# Build tests
cd build
make

# Run all tests
ctest --verbose

# Run specific test suites
./test_dialog
./test_compression
./statistics_test
./simple_test
```

## Development Tools

### Statistics Analysis Tool

```bash
# Analyze performance statistics
./statistics_tool --help
./statistics_tool --summary
./statistics_tool --backend zstd --detailed
```

### Package Management Tool

```bash
# Package management
./gdkg_tool --help
./gdkg_tool create --input dialog.yaml --output package.gdkg
./gdkg_tool extract --input package.gdkg --output extracted/
```

## Development

### Code Style

- Follow the existing code style
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions small and focused
- Use RAII and modern C++ features

### Adding New Features

1. Add source files to `src/`
2. Add headers to `include/goethe/`
3. Update `CMakeLists.txt` with new files
4. Add tests in `src/tests/`
5. Update documentation
6. Add statistics tracking if applicable

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

### Statistics System

The statistics system provides comprehensive performance monitoring:

- **Thread-safe**: Atomic operations for concurrent access
- **Real-time metrics**: Compression ratios, throughput, success rates
- **Per-backend tracking**: Individual statistics for each compression backend
- **Global aggregation**: Combined statistics across all backends
- **Analysis tools**: Command-line tool for detailed analysis

### Benefits

- **Extensibility**: Easy to add new compression algorithms
- **Flexibility**: Can switch backends at runtime
- **Maintainability**: Clean separation of concerns
- **Performance**: Optimized for each algorithm with monitoring
- **Reliability**: Graceful fallbacks and error handling
- **Usability**: Multiple levels of abstraction
- **Observability**: Comprehensive performance tracking

## Documentation

- **README.md**: This file - project overview and quick start
- **docs/ARCHITECTURE.md**: Detailed architecture documentation
- **docs/QUICKSTART.md**: Step-by-step getting started guide
- **docs/STATISTICS.md**: Statistics system documentation
- **docs/CI_CD.md**: CI/CD pipeline documentation
- **docs/SUMMARY.md**: Project summary and status

## License

This project is open source. See LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Update documentation
7. Submit a pull request

## Roadmap

- [x] Add comprehensive statistics tracking
- [x] Implement Google Test integration
- [x] Add command-line analysis tools
- [ ] Add LZ4 compression backend
- [ ] Add Zlib compression backend
- [ ] Implement package system with encryption
- [ ] Create GUI tools
- [ ] Add more dialog formats (JSON, XML)
- [ ] Add visual dialog editor
