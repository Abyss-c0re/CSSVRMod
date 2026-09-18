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
  ASSERT_TRUE(Menu3d_ApplyClick(&m, (int)MenuRow::Hand, 1));
  ASSERT_TRUE(m.left_handed);
  ASSERT_TRUE(Menu3d_ApplyClick(&m, (int)MenuRow::Hand, 1));
  ASSERT_FALSE(m.left_handed);
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

TEST(menu3d_grip_moves_panel) {
  Menu3d m;
  ASSERT_NEAR(m.pos.y, kMenuY, 0.001);
  ASSERT_NEAR(m.pos.z, kMenuZ, 0.001);
  const Vec3 hand0{0.f, kMenuY, 0.f};
  const auto mid = Menu3d_RayHit(hand0, {0.f, 0.f, -1.f}, m.pos);
  ASSERT_TRUE(mid.on_quad);
  ASSERT_TRUE(Menu3d_GripTick(&m, true, hand0, mid.on_quad));
  ASSERT_TRUE(m.gripping);
  const Vec3 hand1{0.2f, kMenuY + 0.1f, 0.1f};
  ASSERT_TRUE(Menu3d_GripTick(&m, true, hand1, true));
  ASSERT_NEAR(m.pos.x, 0.2f, 0.001);
  ASSERT_NEAR(m.pos.y, kMenuY + 0.1f, 0.001);
  ASSERT_NEAR(m.pos.z, kMenuZ + 0.1f, 0.001);
  ASSERT_FALSE(Menu3d_GripTick(&m, false, hand1, true));
  ASSERT_FALSE(m.gripping);
  ASSERT_NEAR(m.pos.x, 0.2f, 0.001);

  Menu3d idle;
  ASSERT_FALSE(Menu3d_GripTick(&idle, true, {0.f, 0.f, 0.f}, false));
  ASSERT_FALSE(idle.gripping);
  ASSERT_NEAR(idle.pos.z, kMenuZ, 0.001);

  Menu3d far;
  far.pos = {2.f, kMenuY, kMenuZ};
  ASSERT_FALSE(Menu3d_RayHit({0.f, kMenuY, 0.f}, {0.f, 0.f, -1.f}, far.pos).on_quad);
  ASSERT_TRUE(Menu3d_RayHit({2.f, kMenuY, 0.f}, {0.f, 0.f, -1.f}, far.pos).on_quad);

  Menu3d c;
  ASSERT_TRUE(Menu3d_GripTick(&c, true, {0.f, kMenuY, 0.f}, true));
  Menu3d_GripTick(&c, true, {0.f, 10.f, 5.f}, true);
  ASSERT_TRUE(c.pos.y <= 2.61f);
  ASSERT_TRUE(c.pos.z <= -0.29f);
}

TEST(menu3d_reset_pose) {
  Menu3d m;
  m.pos = {1.f, 2.f, -2.f};
  m.yaw = 0.7f;
  m.gripping = true;
  m.grip_off = {1.f, 0.f, 0.f};
  Menu3d_ResetPose(&m);
  ASSERT_NEAR(m.pos.x, 0.f, 0.001);
  ASSERT_NEAR(m.pos.y, kMenuY, 0.001);
  ASSERT_NEAR(m.pos.z, kMenuZ, 0.001);
  ASSERT_NEAR(m.yaw, 0.f, 0.001);
  ASSERT_FALSE(m.gripping);

  const auto title = Menu3d_RayHit({0.f, kMenuY + kMenuH * 0.42f, 0.f}, {0.f, 0.f, -1.f});
  ASSERT_TRUE(title.on_quad);
  ASSERT_FALSE(title.hit);
  ASSERT_TRUE(Menu3d_TitleHit(title));
  const auto mid = Menu3d_RayHit({0.f, kMenuY, 0.f}, {0.f, 0.f, -1.f});
  ASSERT_TRUE(mid.hit);
  ASSERT_FALSE(Menu3d_TitleHit(mid));
}

TEST(menu3d_title_hot) {
  Menu3d m;
  ASSERT_FALSE(Menu3d_TitleHot(m));
  const auto title = Menu3d_RayHit({0.f, kMenuY + kMenuH * 0.42f, 0.f}, {0.f, 0.f, -1.f});
  Menu3d_SetCursor(&m, title);
  ASSERT_TRUE(Menu3d_TitleHot(m));
  unsigned char pix[256 * 96 * 4];
  Menu3d cold;
  Menu3d_Raster(pix, 256, 96, cold);
  auto hotbar = [&](const unsigned char* p) {
    int n = 0;
    for (int i = 0; i < 256 * 96; ++i)
      if (p[i * 4 + 0] >= 190 && p[i * 4 + 1] < 60 && p[i * 4 + 2] < 80) n++;
    return n;
  };
  ASSERT_EQ(hotbar(pix), 0);
  Menu3d_Raster(pix, 256, 96, m);
  ASSERT_TRUE(hotbar(pix) > 40);
  const auto mid = Menu3d_RayHit({0.f, kMenuY, 0.f}, {0.f, 0.f, -1.f});
  Menu3d_SetCursor(&m, mid);
  ASSERT_FALSE(Menu3d_TitleHot(m));
}

TEST(menu3d_home_hint) {
  Menu3d m;
  ASSERT_FALSE(Menu3d_OffHome(m));
  m.pos.x = 0.2f;
  ASSERT_TRUE(Menu3d_OffHome(m));
  unsigned char pix[256 * 96 * 4];
  Menu3d home;
  Menu3d_Raster(pix, 256, 96, home);
  auto hint = [&](const unsigned char* p) {
    int n = 0;
    for (int i = 0; i < 256 * 96; ++i)
      if (p[i * 4 + 0] > 80 && p[i * 4 + 0] < 160 && p[i * 4 + 1] > 180 && p[i * 4 + 2] > 200) n++;
    return n;
  };
  ASSERT_EQ(hint(pix), 0);
  home.pos.x = 0.5f;
  Menu3d_Raster(pix, 256, 96, home);
  ASSERT_TRUE(hint(pix) > 8);
}

TEST(menu3d_primary_hand) {
  ASSERT_EQ(Menu3d_PrimaryHand(false), 1);
  ASSERT_EQ(Menu3d_PrimaryHand(true), 0);
  ASSERT_NEAR(Menu3d_HandAxis(false, 0.1f, 0.9f), 0.9f, 0.001);
  ASSERT_NEAR(Menu3d_HandAxis(true, 0.1f, 0.9f), 0.1f, 0.001);
  Menu3d m;
  ASSERT_FALSE(m.left_handed);
  m.left_handed = true;
  ASSERT_EQ(Menu3d_PrimaryHand(m.left_handed), 0);
}

TEST(menu3d_faces_hmd_yaw) {
  const Vec3 panel{0.f, kMenuY, kMenuZ};
  const Vec3 hmd0{0.f, 1.6f, 0.f};
  ASSERT_NEAR(Menu3d_FaceYaw(panel, hmd0), 0.f, 0.01);
  const float left = Menu3d_FaceYaw({-1.f, kMenuY, kMenuZ}, hmd0);
  ASSERT_TRUE(left > 0.4f && left < 0.9f);
  const float right = Menu3d_FaceYaw({1.f, kMenuY, kMenuZ}, hmd0);
  ASSERT_TRUE(right < -0.4f && right > -0.9f);
  const Menu3dQuat id = Menu3d_YawQuat(0.f);
  ASSERT_NEAR(id.x, 0.f, 0.001);
  ASSERT_NEAR(id.y, 0.f, 0.001);
  ASSERT_NEAR(id.w, 1.f, 0.001);

  const float yaw90 = 1.5707963f;
  const Vec3 c{0.f, kMenuY, kMenuZ};
  const auto side = Menu3d_RayHit({1.f, kMenuY, kMenuZ}, {-1.f, 0.f, 0.f}, c, yaw90);
  ASSERT_TRUE(side.on_quad);
  ASSERT_NEAR(side.u, 0.5f, 0.02);
  ASSERT_NEAR(side.v, 0.5f, 0.02);
  ASSERT_FALSE(Menu3d_RayHit({0.f, kMenuY, 0.f}, {0.f, 0.f, -1.f}, c, yaw90).on_quad);

  Menu3d m;
  m.pos = {-1.f, kMenuY, kMenuZ};
  Menu3d_FaceHmd(&m, hmd0);
  ASSERT_NEAR(m.yaw, left, 0.001);
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
  ASSERT_STREQ(Menu3d_RowLabel(4), "HAND");
  ASSERT_STREQ(Menu3d_RowLabel(5), "VR");
  ASSERT_STREQ(Menu3d_RowLabel(6), "DONE");
  ASSERT_EQ(kMenuRows, 7);
  Menu3d m;
  m.calib.eyescale = 0.20f;
  char val[16];
  Menu3d_RowValue(m, 0, val, 16);
  ASSERT_TRUE(val[0] == '0');
  Menu3d_RowValue(m, (int)MenuRow::Hand, val, 16);
  ASSERT_STREQ(val, "R");
  m.left_handed = true;
  Menu3d_RowValue(m, (int)MenuRow::Hand, val, 16);
  ASSERT_STREQ(val, "L");
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
