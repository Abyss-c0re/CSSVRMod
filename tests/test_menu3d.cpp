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
