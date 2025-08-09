#ifndef GOETHE_H
#define GOETHE_H

#if defined _WIN32 || defined __CYGWIN__
  #ifdef GOETHE_BUILD_SHARED
    #ifdef __GNUC__
      #define GOETHE_API __attribute__ ((dllexport))
    #else
      #define GOETHE_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define GOETHE_API __attribute__ ((dllimport))
    #else
      #define GOETHE_API __declspec(dllimport)
    #endif
  #endif
#else
  #if __GNUC__ >= 4
    #define GOETHE_API __attribute__ ((visibility ("default")))
  #else
    #define GOETHE_API
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GoetheEngine GoetheEngine;

typedef struct GoetheConfig {
  const char* app_name;
  int width, height, target_fps;
  int flags; /* bitmask: low_power, headless, etc. */
  const char* vfs_mounts_json; /* declarative mounts */
} GoetheConfig;

typedef struct GoetheCaps {
  int gpu_available;      /* 0/1 */
  int render_targets;     /* 0/1 */
  int max_texture_size;   /* px */
  unsigned int cpu_simd;  /* bitmask */
} GoetheCaps;

GOETHE_API GoetheEngine* goethe_create(const GoetheConfig*);
GOETHE_API void          goethe_destroy(GoetheEngine*);
GOETHE_API void          goethe_frame(GoetheEngine*, float dt);

GOETHE_API int           goethe_load_project(GoetheEngine*, const char* manifest_path);
GOETHE_API void          goethe_get_caps(GoetheEngine*, GoetheCaps* out);
GOETHE_API int           goethe_set_renderer(GoetheEngine*, const char* backend_name);
/* "sdl", "sdl_software", "cpu" */

GOETHE_API void          goethe_cmd(const char* command, const char* payload_json);
/* e.g., {"op":"hot_reload","path":"assets/scenes/intro.gsc"} */

#ifdef __cplusplus
}
#endif

#endif /* GOETHE_H */


