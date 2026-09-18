#include "cssvrmod/menu3d.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(menu3d_hit_and_click) {
  ASSERT_EQ(Menu3d_HitRow(0.5f, 0.1f), -1);
  ASSERT_TRUE(Menu3d_HitRow(0.5f, 0.25f) == 0);
  Menu3d m;
  m.calib.eyescale = 0.20f;
  ASSERT_TRUE(Menu3d_ApplyClick(&m, 0, 1));
  ASSERT_NEAR(m.calib.eyescale, 0.25f, 0.001);
  ASSERT_TRUE(Menu3d_ApplyClick(&m, (int)MenuRow::Dismiss, 1));
  ASSERT_TRUE(!m.visible);
}

TEST(menu3d_laser_hits_panel) {
  const Vec3 o{0.f, kMenuY, 0.f};
  const auto mid = Menu3d_RayHit(o, {0.f, 0.f, -1.f});
  ASSERT_TRUE(mid.hit);
  ASSERT_NEAR(mid.u, 0.5f, 0.02);
  ASSERT_NEAR(mid.v, 0.5f, 0.02);
  ASSERT_TRUE(mid.row >= 1 && mid.row <= 3);
  const auto left = Menu3d_RayHit({-0.2f, kMenuY, 0.f}, {0.f, 0.f, -1.f});
  ASSERT_TRUE(left.hit);
  ASSERT_TRUE(left.u < 0.5f);
  const auto high = Menu3d_RayHit({0.f, kMenuY + kMenuH * 0.6f, 0.f}, {0.f, 0.f, -1.f});
  ASSERT_FALSE(high.hit);
  const auto behind = Menu3d_RayHit({0.f, kMenuY, -2.f}, {0.f, 0.f, -1.f});
  ASSERT_FALSE(behind.hit);
  const Vec3 aim = Menu3d_AimFromQuat(0.f, 0.f, 0.f, 1.f);
  ASSERT_NEAR(aim.z, -1.f, 0.001);
  ASSERT_TRUE(mid.on_quad);
  ASSERT_FALSE(high.on_quad);
  ASSERT_FALSE(behind.on_quad);
}

TEST(menu3d_laser_cursor_at_uv) {
  int x = -1, y = -1;
  Menu3d_UvPx(0.5f, 0.5f, 100, 80, &x, &y);
  ASSERT_EQ(x, 50);
  ASSERT_EQ(y, 40);
  Menu3d_UvPx(0.f, 0.f, 64, 48, &x, &y);
  ASSERT_EQ(x, 0);
  ASSERT_EQ(y, 0);
  ASSERT_TRUE(Menu3d_OnQuad(0.f, 1.f));
  ASSERT_FALSE(Menu3d_OnQuad(-0.01f, 0.5f));

  unsigned char pix[128 * 96 * 4];
  Menu3d m;
  Menu3d_Raster(pix, 128, 96, m);
  auto cyan = [&](const unsigned char* p) {
    int n = 0;
    for (int i = 0; i < 128 * 96; ++i)
      if (p[i * 4 + 0] < 40 && p[i * 4 + 1] > 180 && p[i * 4 + 2] > 200) n++;
    return n;
  };
  ASSERT_EQ(cyan(pix), 0);

  const auto mid = Menu3d_RayHit({0.f, kMenuY, 0.f}, {0.f, 0.f, -1.f});
  Menu3d_SetCursor(&m, mid);
  ASSERT_TRUE(m.cursor);
  ASSERT_NEAR(m.cu, 0.5f, 0.02);
  ASSERT_NEAR(m.cv, 0.5f, 0.02);
  Menu3d_Raster(pix, 128, 96, m);
  ASSERT_TRUE(cyan(pix) > 8);

  int cx = 0, cy = 0;
  Menu3d_UvPx(m.cu, m.cv, 128, 96, &cx, &cy);
  const unsigned char* c = pix + ((size_t)cy * 128 + (size_t)cx) * 4;
  ASSERT_TRUE(c[0] > 200 && c[1] > 200 && c[2] > 200);

  const auto miss = Menu3d_RayHit({0.f, kMenuY + kMenuH * 0.6f, 0.f}, {0.f, 0.f, -1.f});
  Menu3d_SetCursor(&m, miss);
  ASSERT_FALSE(m.cursor);
}

TEST(menu3d_raster_not_empty) {
  unsigned char pix[64 * 48 * 4];
  Menu3d m;
  Menu3d_Raster(pix, 64, 48, m);
  int lit = 0;
  for (int i = 0; i < 64 * 48; ++i)
    if (pix[i * 4 + 0] > 40) lit++;
  ASSERT_TRUE(lit > 20);
}

TEST(menu3d_labels_and_values) {
  ASSERT_STREQ(Menu3d_RowLabel(0), "EYE");
  ASSERT_STREQ(Menu3d_RowLabel(1), "SCALE");
  ASSERT_STREQ(Menu3d_RowLabel(4), "DONE");
  Menu3d m;
  m.calib.eyescale = 0.20f;
  char val[16];
  Menu3d_RowValue(m, 0, val, 16);
  ASSERT_TRUE(val[0] == '0');
  unsigned char tiny[32 * 16 * 4] = {};
  Menu3d_DrawText(tiny, 32, 16, 0, 0, 1, "EYE", 255, 255, 255);
  int lit = 0;
  for (int i = 0; i < 32 * 16; ++i)
    if (tiny[i * 4] > 200) lit++;
  ASSERT_TRUE(lit > 8);
  unsigned char big[320 * 180 * 4];
  Menu3d_Raster(big, 320, 180, m);
  int yellow = 0;
  for (int i = 0; i < 320 * 180; ++i)
    if (big[i * 4 + 0] > 200 && big[i * 4 + 1] > 180 && big[i * 4 + 2] < 120) yellow++;
  ASSERT_TRUE(yellow > 10);
}
