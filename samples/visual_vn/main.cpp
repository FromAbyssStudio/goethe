#include "goethe.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <string>

int main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
    std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  const char* mounts = "{\"mounts\":[{\"path\":\"assets\",\"type\":\"dir\"}]}";
  GoetheConfig cfg = { "Visual VN", 1280, 720, 60, 0, mounts };
  GoetheEngine* eng = goethe_create(&cfg);
  if (!eng) { std::fprintf(stderr, "Failed to create engine\n"); SDL_Quit(); return 1; }

  goethe_set_renderer(eng, "sdl");

  // Minimal SDL window + loop driving goethe_frame and presenting
  SDL_Window* window = SDL_CreateWindow("Visual VN", 1280, 720, SDL_WINDOW_RESIZABLE);
  if (!window) { std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError()); }

  bool running = true;
  uint64_t last = SDL_GetTicksNS();
  while (running) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_EVENT_QUIT) running = false;
      if (ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) running = false;
    }
    uint64_t now = SDL_GetTicksNS();
    float dt = float(now - last) / 1e9f;
    last = now;
    goethe_frame(eng, dt);
    SDL_Delay(1);
  }

  goethe_destroy(eng);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}


