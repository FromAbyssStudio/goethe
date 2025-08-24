#include "goethe/zstd.hpp"

#ifdef GOETHE_ZSTD_AVAILABLE
#include <zstd.h>
#endif
#include <algorithm>
#include <stdexcept>

namespace goethe {

ZstdCompressionBackend::ZstdCompressionBackend()
    : compression_level_(6)
    , options_() {
#ifdef GOETHE_ZSTD_AVAILABLE
    cctx_ = nullptr;
    dctx_ = nullptr;
#endif
    initialize_contexts();
}

ZstdCompressionBackend::~ZstdCompressionBackend() {
#ifdef GOETHE_ZSTD_AVAILABLE
    if (cctx_) {
        ZSTD_freeCCtx(cctx_);
        cctx_ = nullptr;
    }
    if (dctx_) {
        ZSTD_freeDCtx(dctx_);
        dctx_ = nullptr;
    }
#endif
}

void ZstdCompressionBackend::initialize_contexts() {
#ifdef GOETHE_ZSTD_AVAILABLE
    // Create compression context
    cctx_ = ZSTD_createCCtx();
    if (!cctx_) {
        throw CompressionError("Failed to create ZSTD compression context");
    }
    
    // Create decompression context
    dctx_ = ZSTD_createDCtx();
    if (!dctx_) {
        ZSTD_freeCCtx(cctx_);
        cctx_ = nullptr;
        throw CompressionError("Failed to create ZSTD decompression context");
    }
    
    // Set initial compression level
    update_compression_context();
#else
    throw CompressionError("ZSTD library not available");
#endif
}

std::vector<uint8_t> ZstdCompressionBackend::compress(const uint8_t* data, std::size_t size) {
#ifdef GOETHE_ZSTD_AVAILABLE
    validate_input(data, size);
    
    if (size == 0) {
        return {};
    }
    
    // Get compressed size bound
    const size_t compressed_bound = ZSTD_compressBound(size);
    std::vector<uint8_t> compressed(compressed_bound);
    
    // Compress the data
    const size_t compressed_size = ZSTD_compressCCtx(
        cctx_,
        compressed.data(),
        compressed_bound,
        data,
        size,
        compression_level_
    );
    
    check_zstd_error(compressed_size, "compression");
    
    // Resize to actual compressed size
    compressed.resize(compressed_size);
    return compressed;
#else
    throw CompressionError("ZSTD library not available");
#endif
}

std::vector<uint8_t> ZstdCompressionBackend::decompress(const uint8_t* data, std::size_t size) {
#ifdef GOETHE_ZSTD_AVAILABLE
    validate_input(data, size);
    
    if (size == 0) {
        return {};
    }
    
    // Get decompressed size
    const size_t decompressed_size = ZSTD_getFrameContentSize(data, size);
    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
        throw CompressionError("Invalid ZSTD frame");
    }
    if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        throw CompressionError("Unknown decompressed size");
    }
    
    std::vector<uint8_t> decompressed(decompressed_size);
    
    // Decompress the data
    const size_t actual_size = ZSTD_decompressDCtx(
        dctx_,
        decompressed.data(),
        decompressed_size,
        data,
        size
    );
    
    check_zstd_error(actual_size, "decompression");
    
    if (actual_size != decompressed_size) {
        throw CompressionError("Decompressed size mismatch");
    }
    
    return decompressed;
#else
    throw CompressionError("ZSTD library not available");
#endif
}

std::string ZstdCompressionBackend::version() const {
#ifdef GOETHE_ZSTD_AVAILABLE
    return std::to_string(ZSTD_VERSION_MAJOR) + "." + 
           std::to_string(ZSTD_VERSION_MINOR) + "." + 
           std::to_string(ZSTD_VERSION_RELEASE);
#else
    return "not available";
#endif
}

bool ZstdCompressionBackend::is_available() const {
#ifdef GOETHE_ZSTD_AVAILABLE
    return cctx_ != nullptr && dctx_ != nullptr;
#else
    return false;
#endif
}

void ZstdCompressionBackend::set_compression_level(int level) {
#ifdef GOETHE_ZSTD_AVAILABLE
    if (level < ZSTD_minCLevel() || level > ZSTD_maxCLevel()) {
        throw CompressionError("Invalid compression level: " + std::to_string(level));
    }
    compression_level_ = level;
    update_compression_context();
#else
    throw CompressionError("ZSTD library not available");
#endif
}

void ZstdCompressionBackend::set_options(const CompressionOptions& options) {
#ifdef GOETHE_ZSTD_AVAILABLE
    options_ = options;
    update_compression_context();
    update_decompression_context();
#else
    throw CompressionError("ZSTD library not available");
#endif
}

void ZstdCompressionBackend::set_window_log(int window_log) {
#ifdef GOETHE_ZSTD_AVAILABLE
    if (window_log < 0 || window_log > 30) { // ZSTD_WINDOWLOG_MAX is typically 30
        throw CompressionError("Invalid window log: " + std::to_string(window_log));
    }
    options_.window_log = window_log;
    update_compression_context();
#else
    throw CompressionError("ZSTD library not available");
#endif
}

void ZstdCompressionBackend::set_strategy(int strategy) {
#ifdef GOETHE_ZSTD_AVAILABLE
    if (strategy < 0 || strategy > 9) {
        throw CompressionError("Invalid strategy: " + std::to_string(strategy));
    }
    options_.strategy = strategy;
    update_compression_context();
#else
    throw CompressionError("ZSTD library not available");
#endif
}

void ZstdCompressionBackend::set_dictionary(const std::vector<uint8_t>& dictionary) {
#ifdef GOETHE_ZSTD_AVAILABLE
    options_.dictionary = dictionary;
    options_.dictionary_mode = !dictionary.empty();
    update_compression_context();
    update_decompression_context();
#else
    throw CompressionError("ZSTD library not available");
#endif
}

void ZstdCompressionBackend::clear_dictionary() {
#ifdef GOETHE_ZSTD_AVAILABLE
    options_.dictionary.clear();
    options_.dictionary_mode = false;
    update_compression_context();
    update_decompression_context();
#else
    throw CompressionError("ZSTD library not available");
#endif
}

void ZstdCompressionBackend::update_compression_context() {
#ifdef GOETHE_ZSTD_AVAILABLE
    if (!cctx_) return;
    
    // Set compression level
    ZSTD_CCtx_setParameter(cctx_, ZSTD_c_compressionLevel, compression_level_);
    
    // Set window log if specified
    if (options_.window_log > 0) {
        ZSTD_CCtx_setParameter(cctx_, ZSTD_c_windowLog, options_.window_log);
    }
    
    // Set strategy if specified
    if (options_.strategy > 0) {
        ZSTD_CCtx_setParameter(cctx_, ZSTD_c_strategy, options_.strategy);
    }
    
    // Set dictionary if available
    if (options_.dictionary_mode && !options_.dictionary.empty()) {
        ZSTD_CCtx_loadDictionary(cctx_, options_.dictionary.data(), options_.dictionary.size());
    }
#endif
}

void ZstdCompressionBackend::update_decompression_context() {
#ifdef GOETHE_ZSTD_AVAILABLE
    if (!dctx_) return;
    
    // Set dictionary if available
    if (options_.dictionary_mode && !options_.dictionary.empty()) {
        ZSTD_DCtx_loadDictionary(dctx_, options_.dictionary.data(), options_.dictionary.size());
    }
#endif
}

void ZstdCompressionBackend::check_zstd_error(size_t result, const std::string& operation) {
#ifdef GOETHE_ZSTD_AVAILABLE
    if (ZSTD_isError(result)) {
        throw CompressionError("ZSTD " + operation + " failed: " + ZSTD_getErrorName(result));
    }
#else
    (void)result;
    (void)operation;
    throw CompressionError("ZSTD library not available");
#endif
}

} // namespace goethe
