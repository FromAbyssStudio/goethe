# Goethe Dialog System - Quick Start Guide

## Prerequisites

- C++20 compatible compiler (Clang 12+ preferred, GCC 10+, MSVC 2019+)
- CMake 3.20+
- yaml-cpp library
- zstd library (optional, for compression)
- OpenSSL library (optional, for package encryption)
- Google Test (optional, for testing)

## Installation

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake libyaml-cpp-dev libzstd-dev libssl-dev libgtest-dev

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
sudo pacman -S base-devel cmake yaml-cpp zstd openssl gtest

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
brew install cmake yaml-cpp zstd openssl googletest

# Clone and build
git clone <repository-url>
cd goethe
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Basic Usage

### 1. Create a Dialog YAML File

Create `dialog.yaml` using the simple format:

```yaml
id: intro
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

Or use the advanced GOETHE format:

```yaml
kind: dialogue
id: intro
startNode: greeting

nodes:
  - id: greeting
    speaker: alice
    line:
      text: Hello, welcome to our story!
      portrait: { id: alice, mood: happy }
      voice: { clipId: vo_alice_greeting }
    choices:
      - id: continue
        text: Continue
        to: response
  - id: response
    speaker: bob
    line:
      text: Thank you, I'm excited to begin!
      portrait: { id: bob, mood: excited }
    autoAdvanceMs: 2000
```

### 2. C++ Example

Create `main.cpp`:

```cpp
#include <goethe/dialog.hpp>
#include <goethe/manager.hpp>
#include <goethe/statistics.hpp>
#include <iostream>
#include <fstream>

int main() {
    try {
        // Initialize compression manager
        auto& comp_manager = goethe::CompressionManager::instance();
        comp_manager.initialize("zstd"); // or auto-select
        
        // Enable statistics tracking
        auto& stats_manager = goethe::StatisticsManager::instance();
        stats_manager.enable_statistics(true);
        
        // Load dialog from file
        std::ifstream file("dialog.yaml");
        goethe::Dialogue dialogue = goethe::read_dialogue(file);
        
        // Print dialog information
        std::cout << "ID: " << dialogue.id << std::endl;
        std::cout << "Nodes: " << dialogue.nodes.size() << std::endl;
        
        // Iterate through dialog nodes
        for (const auto& node : dialogue.nodes) {
            if (node.speaker) {
                std::cout << *node.speaker << ": " << node.line.text << std::endl;
            } else {
                std::cout << "Narrator: " << node.line.text << std::endl;
            }
        }
        
        // Test compression with statistics
        std::string test_data = "This is some test data for compression";
        std::vector<uint8_t> data(test_data.begin(), test_data.end());
        
        auto compressed = comp_manager.compress(data);
        auto decompressed = comp_manager.decompress(compressed);
        
        // Get performance statistics
        auto stats = stats_manager.get_backend_stats("zstd");
        std::cout << "\nCompression Statistics:" << std::endl;
        std::cout << "Compression ratio: " << stats.average_compression_ratio() << std::endl;
        std::cout << "Success rate: " << stats.success_rate() << std::endl;
        std::cout << "Throughput: " << stats.average_compression_throughput_mbps() << " MB/s" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
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
    if (goethe_dialog_load_from_file(dialog, "dialog.yaml") != 0) {
        fprintf(stderr, "Failed to load dialog file\n");
        goethe_dialog_destroy(dialog);
        return 1;
    }
    
    // Get dialog info
    printf("ID: %s\n", goethe_dialog_get_id(dialog));
    printf("Nodes: %d\n", goethe_dialog_get_node_count(dialog));
    
    // Iterate through nodes
    for (int i = 0; i < goethe_dialog_get_node_count(dialog); i++) {
        GoetheDialogNode* node = goethe_dialog_get_node(dialog, i);
        if (node) {
            const char* speaker = node->speaker ? node->speaker : "Narrator";
            printf("%s: %s\n", speaker, node->line.text);
        }
    }
    
    // Clean up
    goethe_dialog_destroy(dialog);
    return 0;
}
```

### 4. Build and Run

```bash
# Compile
g++ -std=c++20 -I/usr/local/include -L/usr/local/lib main.cpp -lgoethe -lyaml-cpp -lzstd

# Run
./a.out
```

## Testing

### Run All Tests

```bash
cd build
ctest --verbose
```

### Run Specific Test Suites

```bash
# Dialog system tests
./test_dialog

# Compression system tests
./test_compression

# Statistics system tests
./statistics_test

# Basic functionality tests
./test_basic

# Simple integration test
./simple_test
```

## Development Tools

### Statistics Analysis Tool

```bash
# Get help
./statistics_tool --help

# View summary statistics
./statistics_tool --summary

# Detailed statistics for specific backend
./statistics_tool --backend zstd --detailed

# Export statistics to file
./statistics_tool --export stats.json
```

### Package Management Tool

```bash
# Get help
./gdkg_tool --help

# Create a package
./gdkg_tool create --input dialog.yaml --output package.gdkg

# Extract a package
./gdkg_tool extract --input package.gdkg --output extracted/

# List package contents
./gdkg_tool list --input package.gdkg
```

## Advanced Features

### Conditional Logic

```yaml
kind: dialogue
id: conditional_example
startNode: start

nodes:
  - id: start
    speaker: npc
    line:
      text: Have you completed the quest?
    choices:
      - id: yes
        text: Yes, I have
        to: quest_complete
        conditions:
          flag: quest_completed
      - id: no
        text: Not yet
        to: quest_incomplete
        conditions:
          not:
            flag: quest_completed
```

### Effects System

```yaml
nodes:
  - id: give_item
    speaker: merchant
    line:
      text: Here's your item!
    effects:
      - type: SET_FLAG
        target: item_received
        value: true
      - type: QUEST_ADD
        target: main_quest
        value: 1
```

### Voice and Portrait Integration

```yaml
nodes:
  - id: voiced_line
    speaker: protagonist
    line:
      text: This line has voice acting!
      voice:
        clipId: vo_protagonist_greeting
        subtitles: true
        startMs: 0
      portrait:
        id: protagonist
        mood: determined
```

## Troubleshooting

### Common Issues

1. **Missing yaml-cpp**: Install with your package manager
2. **Missing zstd**: Install with your package manager or build without compression
3. **Compiler not found**: Ensure you have a C++20 compatible compiler
4. **Tests not building**: Install Google Test or disable testing

### Build Options

```bash
# Disable testing
cmake -DBUILD_TESTS=OFF ..

# Disable compression
cmake -DBUILD_COMPRESSION=OFF ..

# Set specific compiler
cmake -DCMAKE_CXX_COMPILER=clang++ ..
```

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

## Next Steps

1. **Explore the API**: Check the header files in `include/goethe/`
2. **Read the documentation**: See `docs/` for detailed guides
3. **Run examples**: Try the provided examples and tests
4. **Contribute**: Check the contributing guidelines
5. **Report issues**: Use the issue tracker for bugs and feature requests

## Support

- **Documentation**: Check `docs/` directory
- **Examples**: See `src/tests/` for usage examples
- **Issues**: Use the project issue tracker
- **Discussions**: Join the project discussions
