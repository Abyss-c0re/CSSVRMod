#include "cssvrmod/look.hpp"
#include "cssvrmod/source_if.hpp"
#include "cssvrmod/view_setup.hpp"
#include "test_framework.h"
#include <cstring>

using namespace cssvr;

TEST(look_hmd_overrides_game) {
  Pose hmd;
  hmd.ang = {12.f, 80.f, 0.f};
  hmd.valid = true;
  Ang3 game{0.f, 0.f, 0.f};
  auto d = Look_Decide(hmd, nullptr, false, game);
  ASSERT_TRUE(d.applied);
  ASSERT_STREQ(d.reason, "hmd");
  ASSERT_NEAR(d.angles.y, 80.f, 0.001);
  ASSERT_NEAR(d.angles.p, 12.f, 0.001);
}

TEST(look_invalid_hmd_keeps_game) {
  Pose hmd;
  hmd.ang = {9.f, 9.f, 0.f};
  hmd.valid = false;
  Ang3 game{-5.f, 200.f, 0.f};
  auto d = Look_Decide(hmd, nullptr, false, game);
  ASSERT_FALSE(d.applied);
  ASSERT_STREQ(d.reason, "game");
  ASSERT_NEAR(d.angles.y, 200.f, 0.001);
}

TEST(look_snap_fire_beats_hmd) {
  Pose hmd;
  hmd.ang = {0.f, 0.f, 0.f};
  hmd.valid = true;
  GunPose gun;
  gun.forward = {0.f, 1.f, 0.f};
  gun.valid = true;
  auto d = Look_Decide(hmd, &gun, true, Ang3{1.f, 2.f, 0.f});
  ASSERT_TRUE(d.applied);
  ASSERT_STREQ(d.reason, "snap_fire");
  ASSERT_NEAR(d.angles.y, 90.f, 1.f);
}

static Ang3 g_fake_ang{0.f, 10.f, 0.f};

static void FakeGetAng(void*, Ang3* a) {
  if (a) *a = g_fake_ang;
}
static void FakeSetAng(void*, Ang3* a) {
  if (a) g_fake_ang = *a;
}

TEST(viewangles_sane_and_roundtrip) {
  ASSERT_TRUE(ViewAnglesSane({0.f, 0.f, 0.f}));
  ASSERT_TRUE(ViewAnglesSane({-89.f, 359.f, 0.f}));
  ASSERT_FALSE(ViewAnglesSane({999.f, 0.f, 0.f}));
  ASSERT_FALSE(ViewAnglesSane({0.f, 1e9f, 0.f}));
  int dummy = 0;
  g_fake_ang = {0.f, 10.f, 0.f};
  ASSERT_TRUE(ViewAnglesRoundtripOk(&dummy, FakeGetAng, FakeSetAng));
  ASSERT_NEAR(g_fake_ang.y, 10.f, 0.001); // restored
  EngineIf e;
  e.engine = &dummy;
  e.angles_ok = false;
  ASSERT_FALSE(EngineSetViewAngles(e, {0.f, 45.f, 0.f}));
  e.angles_ok = true;
  e.get_angles_idx = 19;
  e.set_angles_idx = 20;
  // Fake object is not a real vtable — Set must refuse without a valid slot table.
  ASSERT_FALSE(EngineGetViewAngles(e, nullptr));
}

TEST(angles_selftest_miss_toast) {
  EngineAnglesToastIn in;
  ASSERT_FALSE(EngineAngles_ToastDecide(in).should_toast);
  in.probed = true;
  in.have_engine = false;
  auto none = EngineAngles_ToastDecide(in);
  ASSERT_TRUE(none.should_toast);
  ASSERT_FALSE(none.abort_vr);
  ASSERT_STREQ(none.reason, "no_engine");
  ASSERT_STREQ(none.label, "ANG · MISS");
  in.have_engine = true;
  auto fail = EngineAngles_ToastDecide(in);
  ASSERT_TRUE(fail.should_toast);
  ASSERT_STREQ(fail.reason, "selftest_fail");
  ASSERT_TRUE(std::strstr(fail.copy, "yaw") != nullptr);
  in.already_shown = true;
  ASSERT_FALSE(EngineAngles_ToastDecide(in).should_toast);
  in.already_shown = false;
  in.selftest_ok = true;
  auto ok = EngineAngles_ToastDecide(in);
  ASSERT_FALSE(ok.should_toast);
  ASSERT_STREQ(ok.reason, "angles_ok");
}

TEST(look_write_angles_roundtrip) {
  unsigned char blob[0x80] = {};
  ViewSetupFields f;
  Ang3 a{7.5f, 123.f, -3.f};
  ASSERT_TRUE(ViewSetup_WriteAngles(blob, sizeof(blob), f, a));
  Vec3 o;
  Ang3 r;
  float fov = 0.f;
  ASSERT_TRUE(ViewSetup_ReadPose(blob, sizeof(blob), f, &o, &r, &fov));
  ASSERT_NEAR(r.p, 7.5f, 0.0001);
  ASSERT_NEAR(r.y, 123.f, 0.0001);
  ASSERT_NEAR(r.r, -3.f, 0.0001);
}
