#pragma once

#include "backend.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

namespace goethe {

class GOETHE_API CompressionFactory {
public:
    using BackendCreator = std::function<std::unique_ptr<CompressionBackend>()>;
    
    // Singleton pattern for global access
    static CompressionFactory& instance();
    
    // Register a backend type
    void register_backend(const std::string& name, BackendCreator creator);
    
    // Create a backend by name
    std::unique_ptr<CompressionBackend> create_backend(const std::string& name);
    
    // Get available backend names
    std::vector<std::string> get_available_backends() const;
    
    // Auto-select the best available backend
    std::unique_ptr<CompressionBackend> create_best_backend();
    
    // Check if a backend is available
    bool is_backend_available(const std::string& name) const;

private:
    CompressionFactory() = default;
    ~CompressionFactory() = default;
    CompressionFactory(const CompressionFactory&) = delete;
    CompressionFactory& operator=(const CompressionFactory&) = delete;
    
    std::unordered_map<std::string, BackendCreator> backends_;
    
    // Priority order for auto-selection
    static const std::vector<std::string> backend_priority_;
};

// Convenience functions
GOETHE_API std::unique_ptr<CompressionBackend> create_compression_backend(const std::string& name = "");
GOETHE_API std::vector<std::string> get_available_compression_backends();

} // namespace goethe
