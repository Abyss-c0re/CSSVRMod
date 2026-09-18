#include "cssvrmod/banner.hpp"
#include "test_framework.h"
#include <cstring>

using namespace cssvr;

TEST(banner_fail_only) {
  ASSERT_TRUE(Banner_ShouldStamp("no_hmd"));
  ASSERT_TRUE(Banner_ShouldStamp("no_loader"));
  ASSERT_FALSE(Banner_ShouldStamp("session_ok"));
  ASSERT_FALSE(Banner_ShouldStamp("idle"));
  ASSERT_STREQ(Banner_Text("no_hmd"), "NO HMD");
  ASSERT_STREQ(Banner_Text("create_instance"), "NO XR");
  ASSERT_STREQ(Banner_Text("session_ok"), "");
}

TEST(banner_stamp_no_hmd) {
  unsigned char pix[160 * 90 * 4];
  std::memset(pix, 8, sizeof(pix));
  ASSERT_TRUE(Banner_Stamp(pix, 160, 90, false, false, "no_hmd"));
  int crimson = 0, yellow = 0;
  for (int i = 0; i < 160 * 90; ++i) {
    const unsigned char* p = pix + i * 4;
    if (p[0] > 100 && p[1] < 40 && p[2] < 50) crimson++;
    if (p[0] > 200 && p[1] > 180 && p[2] < 120) yellow++;
  }
  ASSERT_TRUE(crimson > 80);
  ASSERT_TRUE(yellow > 20);
  ASSERT_FALSE(Banner_Stamp(pix, 160, 90, false, false, "session_ok"));
}

TEST(banner_bgra_and_flip) {
  unsigned char pix[80 * 40 * 4];
  std::memset(pix, 0, sizeof(pix));
  ASSERT_TRUE(Banner_Stamp(pix, 80, 40, true, true, "no_loader"));
  // flip_y: bar is at the last rows (GL top).
  const int bar = Banner_Height(40);
  int lit = 0;
  for (int y = 40 - bar; y < 40; ++y)
    for (int x = 0; x < 80; ++x)
      if (pix[(y * 80 + x) * 4 + 2] > 80) lit++; // BGRA red in byte 2
  ASSERT_TRUE(lit > 40);
}

TEST(banner_glyphs_no_hmd) {
  unsigned char tiny[64 * 16 * 4] = {};
  Banner_DrawText(tiny, 64, 16, false, false, 0, 0, 1, "NO HMD", 255, 255, 255);
  int lit = 0;
  for (int i = 0; i < 64 * 16; ++i)
    if (tiny[i * 4] > 200) lit++;
  ASSERT_TRUE(lit > 20);
}
