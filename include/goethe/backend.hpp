#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <memory>
#include <stdexcept>

// Include the header that defines GOETHE_API
#include "goethe/dialog.hpp"
#include "goethe/statistics.hpp"

namespace goethe {

// Forward declaration for compression options
struct CompressionOptions;

// Exception for compression errors
class GOETHE_API CompressionError : public std::runtime_error {
public:
    explicit CompressionError(const std::string& message) : std::runtime_error(message) {}
};

class GOETHE_API CompressionBackend {
public:
    virtual ~CompressionBackend() = default;

    // Core compression/decompression methods
    virtual std::vector<uint8_t> compress(const uint8_t* data, std::size_t size) = 0;
    virtual std::vector<uint8_t> decompress(const uint8_t* data, std::size_t size) = 0;

    // Overloaded versions for convenience
    virtual std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    virtual std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
    virtual std::vector<uint8_t> compress(const std::string& data);
    virtual std::vector<uint8_t> decompress_to_string(const uint8_t* data, std::size_t size);

    // Metadata methods
    virtual std::string name() const = 0;
    virtual std::string version() const = 0;
    virtual bool is_available() const = 0;

    // Optional: compression level support
    virtual void set_compression_level(int level) = 0;
    virtual int get_compression_level() const = 0;

    // Optional: compression options
    virtual void set_options(const CompressionOptions& options) = 0;
    virtual CompressionOptions get_options() const = 0;

    // Statistics methods
    virtual void enable_statistics(bool enable = true);
    virtual bool is_statistics_enabled() const;
    virtual BackendStats get_statistics() const;
    virtual void reset_statistics();

protected:
    // Helper method for validation
    void validate_input(const uint8_t* data, std::size_t size) const;
    
    // Statistics tracking helpers
    std::vector<uint8_t> compress_with_statistics(const uint8_t* data, std::size_t size);
    std::vector<uint8_t> decompress_with_statistics(const uint8_t* data, std::size_t size);
    
    // Statistics state
    bool statistics_enabled_ = true;
};

// Compression options structure
struct GOETHE_API CompressionOptions {
    int level = 6;                    // Default compression level
    bool dictionary_mode = false;     // Use dictionary for better compression
    std::vector<uint8_t> dictionary;  // Custom dictionary data
    
    // Zstd-specific options
    int window_log = 0;               // 0 = auto, otherwise 2^window_log
    int strategy = 0;                 // 0 = auto, 1 = fast, 2 = dfast, 3 = greedy, 4 = lazy, 5 = lazy2, 6 = btlazy2, 7 = btopt, 8 = btultra, 9 = btultra2
};

} // namespace goethe
