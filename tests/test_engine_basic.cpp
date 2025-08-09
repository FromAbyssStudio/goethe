#include <gtest/gtest.h>

#include "goethe.h"

static GoetheConfig make_default_config()
{
  GoetheConfig cfg{};
  cfg.app_name = "TestApp";
  cfg.width = 640;
  cfg.height = 360;
  cfg.target_fps = 60;
  cfg.flags = 0;
  cfg.vfs_mounts_json = "{}";
  return cfg;
}

TEST(EngineBasics, CreateAndDestroy)
{
  GoetheConfig cfg = make_default_config();
  GoetheEngine* e = goethe_create(&cfg);
  ASSERT_NE(e, nullptr);
  goethe_destroy(e);
}

TEST(EngineBasics, RendererSelection)
{
  GoetheConfig cfg = make_default_config();
  GoetheEngine* e = goethe_create(&cfg);
  ASSERT_NE(e, nullptr);

  EXPECT_EQ(0, goethe_set_renderer(e, "cpu"));
  EXPECT_EQ(0, goethe_set_renderer(e, "sdl"));
  EXPECT_EQ(0, goethe_set_renderer(e, "sdl_software"));
  EXPECT_EQ(-1, goethe_set_renderer(e, "unknown_backend"));

  goethe_destroy(e);
}

TEST(EngineBasics, CapsAreStable)
{
  GoetheConfig cfg = make_default_config();
  GoetheEngine* e = goethe_create(&cfg);
  ASSERT_NE(e, nullptr);

  GoetheCaps caps{};
  goethe_get_caps(e, &caps);

  // Basic invariants for the stub engine
  EXPECT_GE(caps.max_texture_size, 1);

  goethe_destroy(e);
}


