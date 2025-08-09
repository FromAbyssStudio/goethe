#include "sdk/goethe.h"
#include "engine/core/engine.hpp"

#include <new>

extern "C" {

struct GoetheEngine {
  goethe::Engine* impl;
};

GOETHE_API GoetheEngine* goethe_create(const GoetheConfig* cfg)
{
  if (!cfg) return nullptr;
  GoetheEngine* e = new (std::nothrow) GoetheEngine();
  if (!e) return nullptr;
  e->impl = new (std::nothrow) goethe::Engine(*cfg);
  if (!e->impl) { delete e; return nullptr; }
  return e;
}

GOETHE_API void goethe_destroy(GoetheEngine* e)
{
  if (!e) return;
  delete e->impl;
  delete e;
}

GOETHE_API void goethe_frame(GoetheEngine* e, float dt)
{
  if (!e || !e->impl) return;
  e->impl->tick(dt);
}

GOETHE_API int goethe_load_project(GoetheEngine* e, const char* manifest_path)
{
  if (!e || !e->impl) return -1;
  return e->impl->loadProject(manifest_path);
}

GOETHE_API void goethe_get_caps(GoetheEngine* e, GoetheCaps* out)
{
  if (!e || !e->impl) return;
  e->impl->getCaps(out);
}

GOETHE_API int goethe_set_renderer(GoetheEngine* e, const char* backend_name)
{
  if (!e || !e->impl) return -1;
  return e->impl->setRenderer(backend_name);
}

GOETHE_API void goethe_cmd(const char* /*command*/, const char* /*payload_json*/)
{
  // Stub command channel for now
}

} // extern "C"


