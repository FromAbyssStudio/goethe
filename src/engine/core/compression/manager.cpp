#include "goethe/manager.hpp"
#include "goethe/factory.hpp"
#include "goethe/register_backends.hpp"
#include <stdexcept>

namespace goethe {

CompressionManager& CompressionManager::instance() {
    static CompressionManager instance;
    return instance;
}

void CompressionManager::initialize(const std::string& backend_name) {
    // Register all available backends
    register_compression_backends();

    // Create the backend
    if (backend_name.empty()) {
        backend_ = CompressionFactory::instance().create_best_backend();
    } else {
        backend_ = CompressionFactory::instance().create_backend(backend_name);
    }

    initialized_ = true;
}

std::vector<uint8_t> CompressionManager::compress(const uint8_t* data, std::size_t size) {
    if (!initialized_) {
        throw CompressionError("CompressionManager not initialized");
    }
    return backend_->compress(data, size);
}

std::vector<uint8_t> CompressionManager::decompress(const uint8_t* data, std::size_t size) {
    if (!initialized_) {
        throw CompressionError("CompressionManager not initialized");
    }
    return backend_->decompress(data, size);
}

std::vector<uint8_t> CompressionManager::compress(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return {};
    }
    return compress(data.data(), data.size());
}

std::vector<uint8_t> CompressionManager::decompress(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return {};
    }
    return decompress(data.data(), data.size());
}

std::vector<uint8_t> CompressionManager::compress(const std::string& data) {
    if (data.empty()) {
        return {};
    }
    return compress(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

std::string CompressionManager::decompress_to_string(const uint8_t* data, std::size_t size) {
    auto decompressed = decompress(data, size);
    return std::string(reinterpret_cast<const char*>(decompressed.data()), decompressed.size());
}

std::string CompressionManager::decompress_to_string(const std::vector<uint8_t>& data) {
    auto decompressed = decompress(data);
    return std::string(reinterpret_cast<const char*>(decompressed.data()), decompressed.size());
}

void CompressionManager::set_compression_level(int level) {
    if (!initialized_) {
        throw CompressionError("CompressionManager not initialized");
    }
    backend_->set_compression_level(level);
}

int CompressionManager::get_compression_level() const {
    if (!initialized_) {
        throw CompressionError("CompressionManager not initialized");
    }
    return backend_->get_compression_level();
}

void CompressionManager::set_options(const CompressionOptions& options) {
    if (!initialized_) {
        throw CompressionError("CompressionManager not initialized");
    }
    backend_->set_options(options);
}

CompressionOptions CompressionManager::get_options() const {
    if (!initialized_) {
        throw CompressionError("CompressionManager not initialized");
    }
    return backend_->get_options();
}

std::string CompressionManager::get_backend_name() const {
    if (!initialized_) {
        return "uninitialized";
    }
    return backend_->name();
}

std::string CompressionManager::get_backend_version() const {
    if (!initialized_) {
        return "unknown";
    }
    return backend_->version();
}

bool CompressionManager::is_initialized() const {
    return initialized_;
}

void CompressionManager::switch_backend(const std::string& backend_name) {
    // Register backends if not already done
    register_compression_backends();

    try {
        // Try to create new backend
        backend_ = CompressionFactory::instance().create_backend(backend_name);
        initialized_ = true;
    } catch (const CompressionError&) {
        // If backend creation fails, keep the current backend
        // This allows graceful handling of invalid backend names
    }
}

// Statistics methods
void CompressionManager::enable_statistics(bool enable) {
    if (initialized_) {
        backend_->enable_statistics(enable);
    }
    StatisticsManager::instance().enable_statistics(enable);
}

bool CompressionManager::is_statistics_enabled() const {
    return StatisticsManager::instance().is_statistics_enabled();
}

BackendStats CompressionManager::get_statistics() const {
    if (!initialized_) {
        return BackendStats{};
    }
    return backend_->get_statistics();
}

BackendStats CompressionManager::get_global_statistics() const {
    return StatisticsManager::instance().get_global_stats();
}

void CompressionManager::reset_statistics() {
    if (initialized_) {
        backend_->reset_statistics();
    }
}

void CompressionManager::reset_global_statistics() {
    StatisticsManager::instance().reset_all_stats();
}

std::string CompressionManager::export_statistics_json() const {
    return StatisticsManager::instance().export_json();
}

std::string CompressionManager::export_statistics_csv() const {
    return StatisticsManager::instance().export_csv();
}

// Global convenience functions
std::vector<uint8_t> compress_data(const uint8_t* data, std::size_t size, const std::string& backend) {
    auto& manager = CompressionManager::instance();
    if (!manager.is_initialized()) {
        manager.initialize(backend);
    }
    return manager.compress(data, size);
}

std::vector<uint8_t> decompress_data(const uint8_t* data, std::size_t size, const std::string& backend) {
    auto& manager = CompressionManager::instance();
    if (!manager.is_initialized()) {
        manager.initialize(backend);
    }
    return manager.decompress(data, size);
}

} // namespace goethe
