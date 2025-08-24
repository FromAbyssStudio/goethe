#pragma once

#include "backend.hpp"
#include "statistics.hpp"
#include <memory>
#include <string>

namespace goethe {

class GOETHE_API CompressionManager {
public:
    // Singleton pattern
    static CompressionManager& instance();
    
    // Initialize with specific backend or auto-select
    void initialize(const std::string& backend_name = "");
    
    // High-level compression/decompression methods
    std::vector<uint8_t> compress(const uint8_t* data, std::size_t size);
    std::vector<uint8_t> decompress(const uint8_t* data, std::size_t size);
    
    // Convenience overloads
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
    std::vector<uint8_t> compress(const std::string& data);
    std::string decompress_to_string(const uint8_t* data, std::size_t size);
    std::string decompress_to_string(const std::vector<uint8_t>& data);
    
    // Configuration
    void set_compression_level(int level);
    int get_compression_level() const;
    void set_options(const CompressionOptions& options);
    CompressionOptions get_options() const;
    
    // Information
    std::string get_backend_name() const;
    std::string get_backend_version() const;
    bool is_initialized() const;
    
    // Switch backends
    void switch_backend(const std::string& backend_name);
    
    // Statistics methods
    void enable_statistics(bool enable = true);
    bool is_statistics_enabled() const;
    BackendStats get_statistics() const;
    BackendStats get_global_statistics() const;
    void reset_statistics();
    void reset_global_statistics();
    std::string export_statistics_json() const;
    std::string export_statistics_csv() const;

private:
    CompressionManager() = default;
    ~CompressionManager() = default;
    CompressionManager(const CompressionManager&) = delete;
    CompressionManager& operator=(const CompressionManager&) = delete;
    
    std::unique_ptr<CompressionBackend> backend_;
    bool initialized_ = false;
};

// Global convenience functions
GOETHE_API std::vector<uint8_t> compress_data(const uint8_t* data, std::size_t size, const std::string& backend = "");
GOETHE_API std::vector<uint8_t> decompress_data(const uint8_t* data, std::size_t size, const std::string& backend = "");

} // namespace goethe
