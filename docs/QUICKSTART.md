# Goethe Dialog System - Quick Start Guide

## Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.20+
- yaml-cpp library
- zstd library (optional, for compression)

## Installation

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake libyaml-cpp-dev libzstd-dev

# Clone and build
git clone <repository-url>
cd goethe
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Arch Linux

```bash
# Install dependencies
sudo pacman -S base-devel cmake yaml-cpp zstd

# Clone and build
git clone <repository-url>
cd goethe
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS

```bash
# Install dependencies
brew install cmake yaml-cpp zstd

# Clone and build
git clone <repository-url>
cd goethe
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Basic Usage

### 1. Create a Dialog YAML File

Create `dialog.yaml`:

```yaml
dialogue_id: intro
title: Introduction
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

### 2. C++ Example

Create `main.cpp`:

```cpp
#include <goethe/dialog.hpp>
#include <goethe/manager.hpp>
#include <iostream>
#include <fstream>

int main() {
    try {
        // Load dialog from file
        std::ifstream file("dialog.yaml");
        goethe::Dialogue dialog = goethe::read_dialog(file);
        
        // Print dialog information
        std::cout << "Title: " << dialog.title << std::endl;
        std::cout << "Lines: " << dialog.lines.size() << std::endl;
        
        // Print each dialog line
        for (const auto& line : dialog.lines) {
            std::cout << line.character << ": " << line.phrase << std::endl;
        }
        
        // Test compression
        auto& comp_manager = goethe::CompressionManager::instance();
        comp_manager.initialize("zstd");
        
        std::string test_data = "Hello, this is a test string for compression!";
        std::vector<uint8_t> data(test_data.begin(), test_data.end());
        
        auto compressed = comp_manager.compress(data.data(), data.size());
        auto decompressed = comp_manager.decompress(compressed.data(), compressed.size());
        
        std::cout << "Original size: " << data.size() << " bytes" << std::endl;
        std::cout << "Compressed size: " << compressed.size() << " bytes" << std::endl;
        std::cout << "Compression ratio: " << (100.0 * compressed.size() / data.size()) << "%" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### 3. C Example

Create `main.c`:

```c
#include <goethe/goethe_dialog.h>
#include <stdio.h>

int main() {
    // Create dialog object
    GoetheDialog* dialog = goethe_dialog_create();
    if (!dialog) {
        fprintf(stderr, "Failed to create dialog object\n");
        return 1;
    }
    
    // Load from YAML file
    int result = goethe_dialog_load_from_file(dialog, "dialog.yaml");
    if (result != 0) {
        fprintf(stderr, "Failed to load dialog file\n");
        goethe_dialog_destroy(dialog);
        return 1;
    }
    
    // Print dialog information
    printf("Title: %s\n", goethe_dialog_get_title(dialog));
    printf("Lines: %d\n", goethe_dialog_get_line_count(dialog));
    
    // Print each dialog line
    for (int i = 0; i < goethe_dialog_get_line_count(dialog); i++) {
        GoetheDialogLine* line = goethe_dialog_get_line(dialog, i);
        if (line) {
            printf("%s: %s\n", line->character, line->phrase);
        }
    }
    
    // Clean up
    goethe_dialog_destroy(dialog);
    return 0;
}
```

### 4. Build and Run

```bash
# Build your application
g++ -std=c++20 -I/usr/local/include -L/usr/local/lib main.cpp -lgoethe -lyaml-cpp -lzstd

# Run
./a.out
```

## Advanced Usage

### Compression System

```cpp
#include <goethe/manager.hpp>
#include <vector>

// Initialize compression manager
auto& manager = goethe::CompressionManager::instance();
manager.initialize("zstd"); // or auto-select

// Compress data
std::vector<uint8_t> data = { /* your data */ };
auto compressed = manager.compress(data.data(), data.size());

// Decompress data
auto decompressed = manager.decompress(compressed.data(), compressed.size());

// Switch backends
manager.switch_backend("null");
```

### Direct Backend Usage

```cpp
#include <goethe/factory.hpp>

// Create specific backend
auto backend = goethe::create_compression_backend("zstd");
backend->set_compression_level(10);

// Compress data
auto compressed = backend->compress(data.data(), data.size());
auto decompressed = backend->decompress(compressed.data(), compressed.size());
```

### Global Convenience Functions

```cpp
#include <goethe/manager.hpp>

// Global compression functions
auto compressed = goethe::compress_data(data.data(), data.size(), "zstd");
auto decompressed = goethe::decompress_data(compressed.data(), compressed.size(), "zstd");
```

## Testing

Run the built-in tests:

```bash
cd build
./simple_test
```

Expected output:
```
=== Goethe Dialog System Test ===
Dialog loaded successfully
Title: Test Dialog
Lines: 2
Alice: Hello, this is a test!
Bob: It's working great!
Compression test passed
=== All tests passed! ===
```

## Troubleshooting

### Common Issues

1. **yaml-cpp not found**
   ```bash
   # Ubuntu/Debian
   sudo apt install libyaml-cpp-dev
   
   # Arch Linux
   sudo pacman -S yaml-cpp
   
   # macOS
   brew install yaml-cpp
   ```

2. **zstd not found**
   ```bash
   # Ubuntu/Debian
   sudo apt install libzstd-dev
   
   # Arch Linux
   sudo pacman -S zstd
   
   # macOS
   brew install zstd
   ```

3. **CMake version too old**
   ```bash
   # Update CMake
   sudo apt install cmake  # Ubuntu/Debian
   sudo pacman -S cmake    # Arch Linux
   brew install cmake      # macOS
   ```

4. **Compiler not C++20 compatible**
   ```bash
   # Update GCC
   sudo apt install g++-10  # Ubuntu/Debian
   sudo pacman -S gcc       # Arch Linux
   ```

### Debug Build

```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Verbose Output

```bash
cmake -DCMAKE_VERBOSE_MAKEFILE=ON ..
make VERBOSE=1
```

## Next Steps

1. **Read the Architecture Documentation**: `docs/ARCHITECTURE.md`
2. **Explore the API Reference**: Check the header files in `include/goethe/`
3. **Run Examples**: Look at the test files in `src/tests/`
4. **Contribute**: Check the contributing guidelines in the main README

## Support

- **Documentation**: Check the `docs/` directory
- **Issues**: Report bugs on the project's issue tracker
- **Discussions**: Join the project's discussion forum
