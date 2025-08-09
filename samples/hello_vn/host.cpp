#include "goethe.h"
#include <cstdio>

static inline unsigned long long now_us() {
  return 0ULL; // Stub for sample; real host should provide a clock
}

int main() {
  const char* mounts = "{\"mounts\":[{\"path\":\"assets\",\"type\":\"dir\"}]}";
  GoetheConfig cfg = { "SampleVN", 1280, 720, 60, 0, mounts };
  GoetheEngine* eng = goethe_create(&cfg);
  if (!eng) { std::fprintf(stderr, "Failed to create engine\n"); return 1; }

  goethe_load_project(eng, "assets/project.goethe.json");

  for (int i = 0; i < 3; ++i) {
    goethe_frame(eng, 1.0f/60.0f);
  }

  GoetheCaps caps{};
  goethe_get_caps(eng, &caps);

  goethe_destroy(eng);
  return 0;
}


