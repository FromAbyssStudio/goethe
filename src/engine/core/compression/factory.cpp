#include "goethe/factory.hpp"
#include <algorithm>
#include <stdexcept>

namespace goethe {

// Priority order for backend auto-selection (best first)
const std::vector<std::string> CompressionFactory::backend_priority_ = {
    "zstd",  // Best compression ratio and speed
    "lz4",   // Very fast
    "zlib",  // Widely supported
    "null"   // Fallback (no compression)
};

CompressionFactory& CompressionFactory::instance() {
    static CompressionFactory instance;
    return instance;
}

void CompressionFactory::register_backend(const std::string& name, BackendCreator creator) {
    backends_[name] = std::move(creator);
}

std::unique_ptr<CompressionBackend> CompressionFactory::create_backend(const std::string& name) {
    auto it = backends_.find(name);
    if (it == backends_.end()) {
        throw CompressionError("Unknown compression backend: " + name);
    }

    auto backend = it->second();
    if (!backend->is_available()) {
        throw CompressionError("Compression backend '" + name + "' is not available");
    }

    return backend;
}

std::vector<std::string> CompressionFactory::get_available_backends() const {
    std::vector<std::string> available;
    for (const auto& [name, creator] : backends_) {
        auto backend = creator();
        if (backend->is_available()) {
            available.push_back(name);
        }
    }
    return available;
}

std::unique_ptr<CompressionBackend> CompressionFactory::create_best_backend() {
    // Try backends in priority order
    for (const auto& name : backend_priority_) {
        if (is_backend_available(name)) {
            return create_backend(name);
        }
    }

    // If no backend is available, throw an error
    throw CompressionError("No compression backends are available");
}

bool CompressionFactory::is_backend_available(const std::string& name) const {
    auto it = backends_.find(name);
    if (it == backends_.end()) {
        return false;
    }

    auto backend = it->second();
    return backend->is_available();
}

// Convenience functions
std::unique_ptr<CompressionBackend> create_compression_backend(const std::string& name) {
    if (name.empty()) {
        return CompressionFactory::instance().create_best_backend();
    }
    return CompressionFactory::instance().create_backend(name);
}

std::vector<std::string> get_available_compression_backends() {
    return CompressionFactory::instance().get_available_backends();
}

} // namespace goethe
