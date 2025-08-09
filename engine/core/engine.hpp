#pragma once

#include <cstdint>
#include <string>

struct GoetheConfig;
struct GoetheCaps;

namespace goethe {

// Internal C++ engine object. PIMPL can be added later; keep simple for stub.
class Engine final {
public:
  explicit Engine(const GoetheConfig& cfg);
  ~Engine();

  void tick(float dtSeconds);
  int  loadProject(const char* manifestPath);
  void getCaps(GoetheCaps* outCaps) const;
  int  setRenderer(const char* backendName);

private:
  std::string applicationName_;
  int width_ = 0;
  int height_ = 0;
  int targetFps_ = 60;
  int flags_ = 0;
  std::string mountsJson_;

  // Minimal stub capabilities
  bool gpuAvailable_ = false;
  bool renderTargets_ = false;
  int  maxTextureSize_ = 2048;
  uint32_t cpuSimdMask_ = 0u;
};

} // namespace goethe


