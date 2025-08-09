# Goethe Engine — Architectural Description

## Purpose & Scope

**Goethe** is a compact, cross-platform engine for 2D narrative games and visual novels. It is shipped as a **single shared library** (DLL/.so/.dylib) with a strict **C ABI** for maximal host compatibility and console readiness. Projects are **data-only** (assets and manifests); the host application links the library and drives the main loop.

**Primary objectives**

* Visual-novel-first runtime (dialogue, branching, variables, rollback).
* **GPU-optional** rendering: runs fully on CPU; accelerates when hardware permits.
* **SDL 3** for platform services on desktop/dev; SDK shims on consoles.
* **Binaural audio** (HRTF) for VO/ambience with low CPU budget.
* Deterministic simulation, compact saves/replays.
* Clear separation of **code vs assets** via a virtual filesystem (VFS) and pack files.
* CMake build system; Clang/LLVM first across platforms.

**Out of scope (v1)**

* 3D rendering and physics.
* Heavy in-engine editors (use CLI tools + external DCCs).
* Large third-party dependency graph (keep optional/replaceable).

---

## Design Principles

* **Single shared library**: one engine binary; host app is a thin shell.
* **C ABI** externally, C++20 internally (PIMPL; no exceptions/RTTI in core).
* **Determinism** for narrative stepping and replays (engine-owned RNG).
* **Data-driven** assets and manifests; hot-reload on PC builds.
* **Graceful degradation**: render/audio features scale with device capability.
* **Tight memory discipline**: pools/arenas, minimal heap churn, small working set.

---

## High-Level Architecture

```
Host App (C/C++) ──links──► goethe.(dll|so|dylib)
                           ├─ Core (handles, memory, jobs, timers, events)
                           ├─ Platform (SDL3 or SDK shim: window, input, FS, time, audio device)
                           ├─ VFS (mounts: dir/zip/pak; read-only in shipping)
                           ├─ Resource Manager (textures, fonts, sounds, scripts; hot-reload PC)
                           ├─ Render (HAL + backends: SDL3 accelerated/software, CPU raster)
                           ├─ Text (font bake, shaping; MSDF→bitmap cache)
                           ├─ Audio (mixer, streaming, HRTF buses)
                           ├─ Narrative VM (timeline, choices, vars, rollback)
                           ├─ Script VM (Lua or native; deterministic hooks)
                           ├─ UI Widgets (dialogue box, choices, backlog, save/load stubs)
                           └─ Save/Replay (versioned, compact, encrypted per platform)
```

---

## Process & Threading Model

* **Main thread**: engine tick (`goethe_frame`), input collection, narrative VM, render submission, high-level resource ref-counts.
* **Job system**: optional worker pool (2–4 threads) for texture/audio decode, atlas builds, CPU raster scanlines, and HRTF block FFTs.
* **Audio thread**: mixer callback or dedicated thread pushed by SDL3 audio device.
* **I/O thread**: asynchronous reads into staging buffers; pinned for pack file streaming.

Synchronization is limited to lock-free queues and thin fences; heavy locks avoided in the play loop.

---

## Platform Abstraction

* **Primary**: **SDL 3** for windowing, input, timers, haptics, file I/O (dev only), and audio device.
* **Consoles/SDKs**: drop-in `IPlatform` shim implementing the same surface:

  * Window/Surface, Gamepad API, Time, File I/O (title-safe paths), App State (suspend/resume).
* **Headless**: tools and CI use a headless build (no window/audio).

---

## Rendering Subsystem (GPU-Optional)

### Goals

* Always functional on CPU-only systems.
* Prefer hardware acceleration when available (SDL3 accelerated renderer).
* Stable visuals for VN use cases: sprites, layers, text, simple effects.

### Render HAL

```cpp
struct RenderCaps {
  bool gpu_available;
  bool render_targets;
  int  max_texture_size;
  uint32_t cpu_simd;    // bitmask: SSE2|AVX2|NEON
};

struct IRenderer {
  virtual bool init(const RenderCaps&) = 0;
  virtual void shutdown() = 0;
  virtual void begin_frame(int w, int h) = 0;
  virtual void draw_quads(const QuadBatch&) = 0;
  virtual void draw_text(const TextBatch&) = 0;
  virtual void end_frame() = 0;
  virtual TextureHandle upload_texture(ImageView) = 0;
  virtual void destroy_texture(TextureHandle) = 0;
};
```

### Backends

1. **SDL3 Renderer path**

   * Attempt `SDL_RENDERER_ACCELERATED`; fall back to `SDL_RENDERER_SOFTWARE`.
   * Batched geometry via `SDL_RenderGeometry`.
   * Effects: colour/alpha, additive/multiply; wipes/crossfades via intermediate targets (if supported).

2. **CPU Raster path**

   * Premultiplied-alpha RGBA8888 scanline compositor.
   * SIMD kernels (SSE2/AVX2/NEON) for blit, scale (nearest/bilinear), 9-slice, tint.
   * Integer scaling, fixed timestep, 30 FPS low-power mode.

### Text & Fonts

* **Offline MSDF→bitmap**: glyphs baked to atlases at required sizes (build time or first-use).
* **Shaping**: HarfBuzz optional; start with Latin; RTL/CJK in v1.1.
* **Caching**: LRU for rasterised glyph tiles; per-locale font fallback chain.

---

## Audio Subsystem (Binaural-First)

* **Device**: opened via SDL3; internal mixer runs at 48 kHz float32.
* **Voices**: 32–64, routed through buses (Music, SFX, VO, Ambience).
* **Streaming**: Ogg/Opus/PCM; VO streamed from VFS with small pre-roll.
* **DSP**: volume, pan, per-bus EQ; light FDN reverb.
* **HRTF**: SOFA-compatible IRs; convolution on VO/Ambience buses using overlap-save block FFT. Downsample option and voice cap to stay within 1–2% CPU on mid-tier ARM. Fallback: stereo pan.

---

## Narrative System

### Model

* **Scene** → **Nodes** → **Commands** (say, bg, music, choice, goto, flag, wait, effect).
* **State** captures: current node/cmd index, variables/flags, PRNG seed, layer states, audio cursors (logical), pending timers.

### Script Frontends

* Import **Ink/Yarn** at build into compact bytecode, or use **GoetheScript** (native line-based).
* VM is deterministic; no wall-clock queries inside VM; all randomness via engine PRNG.

### Rollback & Backlog

* **Rollback ring** stores periodic diffs (vars, node index, RNG seed).
* **Backlog** records displayed lines and choices; page through in UI.

---

## Scripting & Embedding

* **Embedded Lua 5.4** (toggleable). Bindings limited to:

  * Scene graph (layers, sprites), variables, timers, UI hooks.
  * Audio control, file queries via VFS, platform signals.
* Deterministic hooks only: Lua time/RNG replaced with engine services.
* Externally, expose **C ABI** commands for host control and tool automation.

---

## UI Layer

* Skinnable widgets: dialogue box, choice menu, backlog, save/load stubs.
* Defined as JSON themes + 9-slice images + bitmap fonts.
* Input abstraction: KB/Mouse, Gamepad, Touch; glyph-aware prompts per platform.

---

## Virtual Filesystem (VFS) & Resource Management

### Mounts

* Order-based search across: `assets/`, `patch/`, `dlc/`, `mods/` (dev only), and **pack files** (`.gpak`).
* Shipping builds: read-only packs + writable save partition.

### Pack Format (`.gpak`)

* Header (magic, version), block-compressed data (zstd), file table with offsets/sizes/XXH3 hashes, optional per-pack key (platform-specific).
* Memory-mapped where allowed; otherwise async reads with small cache.

### Resource Manager

* Typed handles (Texture/Sound/Font/Script).
* Lifetime via intrusive ref-counts; descriptor caches keyed by stable IDs.
* Hot-reload on PC: file watchers push reload events; console/devkits via command channel.

---

## Save/Load & Replay

* **Save**: versioned binary blob containing narrative state, variables, PRNG seed, layer descriptors (not texture data), and minimal audio cursors. Optional encryption + MAC per platform policy.
* **Auto-save** before/after choices and scene transitions.
* **Replay**: logs choices and inputs + initial seed; re-simulated deterministically for QA.

---

## Localisation & Accessibility

* Locale packs hold string tables (per scene and UI), voice tags, and font atlases.
* Features: text scaling, dyslexic-friendly font option, high-contrast themes, auto-read mode (TTS hook via platform), input remapping.

---

## Input System

* Unified events from SDL3 (or SDK shim).
* Mappable actions: advance, back, choice up/down, menu, quick-save/load (dev).
* Text input path supports IME for CJK (v1.1).

---

## Build System & Toolchain

### CMake

* Presets for **Clang-first** on Linux/macOS and **clang-cl** on Windows.
* Options:

  * `GOETHE_BACKEND_SDL3` (ON), `GOETHE_BACKEND_CPU` (ON)
  * `GOETHE_WITH_LUA` (ON), `GOETHE_WITH_HARFBUZZ` (optional), `GOETHE_WITH_HRTF` (ON)
  * `GOETHE_BUILD_SHARED` (ON), `GOETHE_BUILD_TOOLS` (ON), `GOETHE_BUILD_TESTS` (ON)
* Visibility hidden by default; exports via `GOETHE_API`.
* Sanitizers in Debug/RelWithDebInfo; ThinLTO for Release where available.

### Compilers

* **Default**: Clang/LLVM (Win: clang-cl targeting MSVC ABI; Linux: clang; macOS: Apple Clang).
* Tier-1 fallbacks: MSVC (latest), GCC (latest stable) in CI matrices.

---

## Public C ABI (excerpt)

```c
typedef struct GoetheEngine GoetheEngine;

typedef struct GoetheConfig {
  const char* app_name;
  int width, height, target_fps;
  int flags;                 /* bitmask: low_power, headless, etc. */
  const char* vfs_mounts_json; /* declarative mounts */
} GoetheConfig;

typedef struct GoetheCaps {
  int gpu_available;      /* 0/1 */
  int render_targets;     /* 0/1 */
  int max_texture_size;   /* px */
  unsigned cpu_simd;      /* bitmask */
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
```

---

## Configuration & Project Manifest

`project.goethe.json` (example):

```json
{
  "title": "Rooftop Story",
  "entry_scene": "scenes/intro.gsc",
  "locales": ["en-GB","pt-BR"],
  "renderer": {"target_fps": 60, "low_power": false},
  "audio":    {"sample_rate": 48000, "binaural_on": true, "hrtf": "hrtf/default.sofa"},
  "mounts": [
    {"path":"assets","type":"dir"},
    {"path":"dlc","type":"dir","optional":true}
  ]
}
```

---

## Asset Pipeline & Tools (`goethec`)

* `goethec pack` → build `.gpak` from directories with manifest, compression level, and whitelist.
* `goethec atlas` → sprite packing; emits `.atlas.json` + sheets.
* `goethec font` → bake MSDF→bitmap atlases per locale/size.
* `goethec ink|yarn` → compile scripts into bytecode for the Narrative VM.
* `goethec validate` → static checks (missing assets, string keys, locale coverage).
* All tools run headless; CI uses them to validate content and determinism.

---

## Testing & QA Strategy

* **Unit tests**: VFS, pack reader, narrative VM stepping, RNG determinism.
* **Golden frames**: record frame hashes for canonical scenes on **CPU** and **SDL software** backends; catch visual drift.
* **Audio checks**: per-bus envelope/RMS comparisons with HRTF on/off.
* **Fuzzing**: narrative bytecode loader, text parser, pack index reader.
* **Sanitizers**: ASan/UBSan on nightly builds.
* **Replay tests**: deterministic re-sim from recorded inputs and seeds.

---

## Diagnostics & Telemetry (optional)

* In-engine overlays (dev): frame time, draw batches, texture cache usage, audio voice counts.
* Event log channels (narrative steps, save/load, resource misses).
* Optional telemetry hook (host-provided callback) for anonymised metrics.

---

## Performance Targets & Low-Power Strategy

* **Low-power (ARM A53-class @ 30 FPS)**:

  * Update ≤ 2 ms, Render ≤ 8 ms, Audio ≤ 1 ms.
* Techniques:

  * Atlas batching, integer scaling, limited shader set (when GPU), half-res offscreen for transitions, capped simultaneous HRTF convolving voices, pre-decoded VO chunks, fixed timestep, aggressive glyph caching.

---

## Security, Compliance, TRCs

* **No self-modifying/JIT** code; deterministic VM.
* **Save path separation**: writable user area distinct from read-only packs.
* **Graceful suspend/resume** handling.
* **Controller** compliance: glyphs and remapping per platform.
* **Content** safety: manifests and pack signatures optional for tamper detection.

---

## Packaging & Distribution

* Deliverables:

  * `goethe.(dll|so|dylib)` + `sdk/` headers + `GoetheConfig.cmake` for consumers.
  * Tools: `goethec` binary.
  * Sample: “Hello VN” project (assets + manifest).
* Shipping packs are read-only `.gpak`; patch/DLC packs mount above base.

---

## Risks & Mitigations

* **Text shaping complexity** → Stage rollout: Latin first; add HarfBuzz, RTL, CJK in v1.1; pre-baked fonts for constrained devices.
* **HRTF CPU cost** → Limit to VO/Ambience buses; cap concurrent convolving voices; allow downsample; SIMD FFT.
* **SDL3 availability on some targets** → Provide SDK shims; keep HAL narrow.
* **Determinism drift** → Golden tests, single PRNG source, strict VM rules.

---

## Roadmap (post-MVP)

* RTL/CJK shaping + IME integration.
* Subtitle/closed-caption tracks and audio description hooks.
* Automated localisation QA tools (missing keys, overflow detection).
* Optional GL/Metal/D3D11 backend for more effects while keeping CPU path.
* Scripting sandbox profiler (per-scene budget alerts).

---

## Appendix A — Minimal Host Loop (C++)

```cpp
#include "goethe.h"

int main() {
  GoetheConfig cfg = { "SampleVN", 1280, 720, 60, 0, "{\"mounts\":[{\"path\":\"assets\",\"type\":\"dir\"}]}" };
  GoetheEngine* eng = goethe_create(&cfg);
  goethe_load_project(eng, "assets/project.goethe.json");

  bool running = true;
  uint64_t last = now_us();
  while (running) {
    uint64_t cur = now_us();
    float dt = float(cur - last) / 1e6f; last = cur;
    /* pump SDL3 events → goethe_cmd(...) for input/signals if desired */
    goethe_frame(eng, dt);
  }
  goethe_destroy(eng);
}
```

---

## Appendix B — Directory Layout (suggested)

```
/engine
  /core        (handles, memory, jobs, vfs)
  /platform    (sdl3, sdk_shims/*)
  /render      (iface, backend_sdl3, backend_cpu)
  /audio       (mixer, hrtf, decoders)
  /text        (font_bake, shaping)
  /narrative   (vm, save, rollback)
  /script      (lua_vm, bindings)
  /ui          (widgets, theming)
  /tools       (shared tool libs)
/sdk           (public headers)
/tools/goethec (cli)
/samples/hello_vn
/tests
```

This document should drop directly into Codex/Confluence as the authoritative architectural overview. If you want, I can now expand any section into a lower-level design (e.g., narrative bytecode spec, pack file format, or the full public C header).
