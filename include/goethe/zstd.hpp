#pragma once

#include "goethe/backend.hpp"
#include <memory>

// Forward declarations to avoid including zstd.h in header
#ifdef GOETHE_ZSTD_AVAILABLE
struct ZSTD_CCtx_s;
struct ZSTD_DCtx_s;
#endif

namespace goethe {

class ZstdCompressionBackend : public CompressionBackend {
public:
    ZstdCompressionBackend();
    ~ZstdCompressionBackend() override;

    // Core compression/decompression
    std::vector<uint8_t> compress(const uint8_t* data, std::size_t size) override;
    std::vector<uint8_t> decompress(const uint8_t* data, std::size_t size) override;

    // Metadata
    std::string name() const override {
        return "zstd";
    }
    std::string version() const override;
    bool is_available() const override;

    // Compression level (1-22 for zstd)
    void set_compression_level(int level) override;
    int get_compression_level() const override {
        return compression_level_;
    }

    // Options
    void set_options(const CompressionOptions& options) override;
    CompressionOptions get_options() const override {
        return options_;
    }

    // Zstd-specific methods
    void set_window_log(int window_log);
    void set_strategy(int strategy);
    void set_dictionary(const std::vector<uint8_t>& dictionary);
    void clear_dictionary();

private:
    // Zstd contexts
#ifdef GOETHE_ZSTD_AVAILABLE
    ZSTD_CCtx_s* cctx_;
    ZSTD_DCtx_s* dctx_;
#endif
    
    // Configuration
    int compression_level_;
    CompressionOptions options_;
    
    // Helper methods
    void initialize_contexts();
    void update_compression_context();
    void update_decompression_context();
    
    // Error handling
    static void check_zstd_error(size_t result, const std::string& operation);
};

} // namespace goethe
