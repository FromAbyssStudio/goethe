#pragma once

#include "goethe/backend.hpp"

namespace goethe {

class NullCompressionBackend : public CompressionBackend {
public:
    NullCompressionBackend() = default;
    ~NullCompressionBackend() override = default;

    // Core compression/decompression (no-op)
    std::vector<uint8_t> compress(const uint8_t* data, std::size_t size) override;
    std::vector<uint8_t> decompress(const uint8_t* data, std::size_t size) override;

    // Metadata
    std::string name() const override {
        return "null";
    }
    std::string version() const override {
        return "1.0.0";
    }
    bool is_available() const override {
        return true;
    }

    // Compression level (ignored for null backend)
    void set_compression_level(int level) override {
        (void)level;
    }
    int get_compression_level() const override {
        return 0;
    }

    // Options (ignored for null backend)
    void set_options(const CompressionOptions& options) override {
        (void)options;
    }
    CompressionOptions get_options() const override {
        return CompressionOptions{};
    }
};

} // namespace goethe
