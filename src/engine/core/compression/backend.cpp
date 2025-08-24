#include "goethe/backend.hpp"
#include <cstring>

namespace goethe {

std::vector<uint8_t> CompressionBackend::compress(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return {};
    }
    return compress_with_statistics(data.data(), data.size());
}

std::vector<uint8_t> CompressionBackend::decompress(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return {};
    }
    return decompress_with_statistics(data.data(), data.size());
}

std::vector<uint8_t> CompressionBackend::compress(const std::string& data) {
    if (data.empty()) {
        return {};
    }
    return compress_with_statistics(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

std::vector<uint8_t> CompressionBackend::decompress_to_string(const uint8_t* data, std::size_t size) {
    auto decompressed = decompress(data, size);
    return decompressed;
}

void CompressionBackend::validate_input(const uint8_t* data, std::size_t size) const {
    if (data == nullptr && size > 0) {
        throw CompressionError("Data pointer is null but size is non-zero");
    }
}

// Statistics methods
void CompressionBackend::enable_statistics(bool enable) {
    statistics_enabled_ = enable;
}

bool CompressionBackend::is_statistics_enabled() const {
    return statistics_enabled_;
}

BackendStats CompressionBackend::get_statistics() const {
    return StatisticsManager::instance().get_backend_stats(name());
}

void CompressionBackend::reset_statistics() {
    StatisticsManager::instance().reset_backend_stats(name());
}

std::vector<uint8_t> CompressionBackend::compress_with_statistics(const uint8_t* data, std::size_t size) {
    if (!statistics_enabled_) {
        return compress(data, size);
    }
    
    StatisticsScope scope(name(), version(), true);
    try {
        auto result = compress(data, size);
        scope.set_sizes(size, result.size());
        scope.set_success(true);
        return result;
    } catch (const std::exception& e) {
        scope.set_sizes(size, 0);
        scope.set_success(false, e.what());
        throw;
    }
}

std::vector<uint8_t> CompressionBackend::decompress_with_statistics(const uint8_t* data, std::size_t size) {
    if (!statistics_enabled_) {
        return decompress(data, size);
    }
    
    StatisticsScope scope(name(), version(), false);
    try {
        auto result = decompress(data, size);
        scope.set_sizes(size, result.size());
        scope.set_success(true);
        return result;
    } catch (const std::exception& e) {
        scope.set_sizes(size, 0);
        scope.set_success(false, e.what());
        throw;
    }
}

} // namespace goethe
