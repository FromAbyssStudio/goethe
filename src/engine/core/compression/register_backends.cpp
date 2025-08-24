#include "goethe/register_backends.hpp"
#include "goethe/factory.hpp"
#include "goethe/null.hpp"
#include "goethe/zstd.hpp"

namespace goethe {

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

} // namespace goethe
