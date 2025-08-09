#include "engine/core/engine.hpp"
#include "sdk/goethe.h"

#include <cstring>

namespace goethe {

Engine::Engine(const GoetheConfig& cfg)
  : applicationName_(cfg.app_name ? cfg.app_name : "Goethe"),
    width_(cfg.width),
    height_(cfg.height),
    targetFps_(cfg.target_fps),
    flags_(cfg.flags),
    mountsJson_(cfg.vfs_mounts_json ? cfg.vfs_mounts_json : "{}")
{
  // TODO: Detect SIMD, GPU, etc. For now, keep defaults.
}

Engine::~Engine() = default;

void Engine::tick(float /*dtSeconds*/)
{
  // Stub tick: no-op
}

int Engine::loadProject(const char* /*manifestPath*/)
{
  // Stub: accept anything
  return 0;
}

void Engine::getCaps(GoetheCaps* outCaps) const
{
  if (!outCaps) return;
  outCaps->gpu_available = gpuAvailable_ ? 1 : 0;
  outCaps->render_targets = renderTargets_ ? 1 : 0;
  outCaps->max_texture_size = maxTextureSize_;
  outCaps->cpu_simd = cpuSimdMask_;
}

int Engine::setRenderer(const char* backendName)
{
  // Accept known strings; otherwise return error.
  if (!backendName) return -1;
  if (std::strcmp(backendName, "sdl") == 0 ||
      std::strcmp(backendName, "sdl_software") == 0 ||
      std::strcmp(backendName, "cpu") == 0) {
    return 0;
  }
  return -1;
}

} // namespace goethe


