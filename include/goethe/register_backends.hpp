#pragma once

#include "goethe/dialog.hpp"

namespace goethe {

// Register all available compression backends with the factory
GOETHE_API void register_compression_backends();

} // namespace goethe
