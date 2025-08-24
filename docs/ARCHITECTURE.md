# Goethe Dialog System - Architecture Documentation

## Overview

The Goethe Dialog System is built with a modular, extensible architecture that separates concerns and provides multiple levels of abstraction. The system consists of two main components:

1. **Dialog System**: Handles YAML-based dialog loading, parsing, and manipulation
2. **Compression System**: Provides flexible compression with multiple backend implementations

## Architecture Principles

### Design Patterns

The system uses several design patterns to achieve flexibility and maintainability:

1. **Strategy Pattern**: For compression algorithms
2. **Factory Pattern**: For creating compression backends
3. **Manager Pattern**: For high-level API access
4. **Singleton Pattern**: For global manager instances

### Separation of Concerns

- **Interface Layer**: Public headers in `include/goethe/`
- **Implementation Layer**: Source files in `src/engine/`
- **API Layer**: C and C++ APIs for different use cases
- **Test Layer**: Test files in `src/tests/`

## Dialog System Architecture

### Core Components

```
Dialog System
├── DialogueLine          # Individual dialog line
├── Dialogue              # Complete dialog structure
├── read_dialog()         # YAML loading function
├── write_dialog()        # YAML writing function
└── C API Wrapper         # C-compatible interface
```

### Data Flow

1. **Input**: YAML file or string
2. **Parsing**: YAML-cpp library parses the input
3. **Conversion**: YAML nodes converted to C++ structures
4. **Access**: Dialog data accessed via C++ or C APIs
5. **Output**: Dialog data serialized back to YAML

### YAML Integration

The dialog system uses yaml-cpp for YAML processing:

- **Loading**: `YAML::Load()` for parsing YAML input
- **Conversion**: Custom `from_yaml()` and `to_yaml()` functions
- **Serialization**: `YAML::Dump()` for output generation

## Compression System Architecture

### Core Components

```
Compression System
├── CompressionBackend    # Abstract interface
├── CompressionFactory    # Backend creation
├── CompressionManager    # High-level API
├── Backend Registry      # Automatic registration
└── Implementations       # Concrete backends
    ├── NullBackend       # No-op compression
    └── ZstdBackend       # Zstd compression
```

### Design Patterns Implementation

#### Strategy Pattern

```cpp
class CompressionBackend {
public:
    virtual std::vector<uint8_t> compress(const uint8_t* data, std::size_t size) = 0;
    virtual std::vector<uint8_t> decompress(const uint8_t* data, std::size_t size) = 0;
    virtual std::string name() const = 0;
    virtual bool is_available() const = 0;
};
```

#### Factory Pattern

```cpp
class CompressionFactory {
public:
    static CompressionFactory& instance();
    void register_backend(const std::string& name, BackendCreator creator);
    std::unique_ptr<CompressionBackend> create_backend(const std::string& name);
    std::unique_ptr<CompressionBackend> create_best_backend();
};
```

#### Manager Pattern

```cpp
class CompressionManager {
public:
    static CompressionManager& instance();
    void initialize(const std::string& backend_name = "");
    std::vector<uint8_t> compress(const uint8_t* data, std::size_t size);
    std::vector<uint8_t> decompress(const uint8_t* data, std::size_t size);
};
```

### Backend Selection Strategy

The system implements a priority-based backend selection:

1. **Zstd**: Best compression ratio and speed
2. **Null**: Fallback for testing or when no compression is needed

### Registration System

Backends are automatically registered at startup:

```cpp
void register_compression_backends() {
    auto& factory = CompressionFactory::instance();
    
    // Register null backend (always available)
    factory.register_backend("null", []() {
        return std::make_unique<NullCompressionBackend>();
    });
    
    // Register zstd backend (if available)
    factory.register_backend("zstd", []() {
        return std::make_unique<ZstdCompressionBackend>();
    });
}
```

## API Design

### C++ API

The C++ API provides high-level, type-safe access:

```cpp
// Dialog API
goethe::Dialogue dialog = goethe::read_dialog(file);
for (const auto& line : dialog.lines) {
    // Process dialog line
}

// Compression API
auto& manager = goethe::CompressionManager::instance();
manager.initialize("zstd");
auto compressed = manager.compress(data);
```

### C API

The C API provides C-compatible interface for integration:

```c
// Dialog API
GoetheDialog* dialog = goethe_dialog_create();
goethe_dialog_load_from_file(dialog, "dialog.yaml");
GoetheDialogLine* line = goethe_dialog_get_line(dialog, 0);

// Compression API
char* compressed = goethe_compress_data(data, size, "zstd");
```

## Error Handling

### Exception-Based (C++)

C++ code uses exceptions for error handling:

```cpp
try {
    auto backend = CompressionFactory::instance().create_backend("zstd");
    auto compressed = backend->compress(data, size);
} catch (const CompressionError& e) {
    // Handle compression error
}
```

### Return Code-Based (C)

C code uses return codes for error handling:

```c
int result = goethe_dialog_load_from_file(dialog, "dialog.yaml");
if (result != 0) {
    // Handle error
}
```

## Memory Management

### RAII (C++)

C++ code uses RAII for automatic resource management:

```cpp
class ZstdCompressionBackend {
private:
    std::unique_ptr<ZSTD_CCtx_s> cctx_;
    std::unique_ptr<ZSTD_DCtx_s> dctx_;
public:
    ~ZstdCompressionBackend() {
        // Automatic cleanup via unique_ptr
    }
};
```

### Manual Management (C)

C code requires manual memory management:

```c
GoetheDialog* dialog = goethe_dialog_create();
// Use dialog
goethe_dialog_destroy(dialog); // Manual cleanup
```

## Performance Considerations

### Compression Performance

- **Zstd**: Optimized for speed and compression ratio
- **Null**: Minimal overhead for testing
- **Context Reuse**: Compression contexts are reused for efficiency

### Memory Usage

- **Streaming**: Large files processed in chunks
- **Buffer Management**: Efficient buffer allocation and deallocation
- **Zero-Copy**: Minimize data copying where possible

## Extensibility

### Adding New Compression Backends

1. Implement the `CompressionBackend` interface
2. Add registration in `register_compression_backends()`
3. Update priority list in `CompressionFactory`
4. Add tests for the new backend

### Adding New Dialog Formats

1. Implement format-specific loading functions
2. Add format detection logic
3. Update the main dialog loading interface
4. Add tests for the new format

## Testing Strategy

### Unit Tests

- Individual component testing
- Interface compliance testing
- Error condition testing

### Integration Tests

- End-to-end workflow testing
- API compatibility testing
- Performance benchmarking

### Backend Testing

- Compression/decompression round-trip testing
- Error handling testing
- Performance comparison testing

## Future Enhancements

### Planned Features

1. **Additional Compression Backends**: LZ4, Zlib, Brotli
2. **Package System**: Secure package creation and management
3. **Encryption**: OpenSSL-based encryption and signing
4. **GUI Tools**: Visual dialog editor
5. **More Formats**: JSON, XML, binary formats

### Architecture Evolution

1. **Plugin System**: Dynamic backend loading
2. **Configuration System**: Runtime configuration management
3. **Logging System**: Comprehensive logging and debugging
4. **Performance Profiling**: Built-in performance monitoring
