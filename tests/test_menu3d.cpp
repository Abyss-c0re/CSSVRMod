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

TEST(menu3d_raster_not_empty) {
  unsigned char pix[64 * 48 * 4];
  Menu3d m;
  Menu3d_Raster(pix, 64, 48, m);
  int lit = 0;
  for (int i = 0; i < 64 * 48; ++i)
    if (pix[i * 4 + 0] > 40) lit++;
  ASSERT_TRUE(lit > 20);
}
