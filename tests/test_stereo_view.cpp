#include "cssvrmod/stereo_view.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(stereo_half_ipd_follows_eyescale) {
  Calib c;
  c.ipd_m = 0.064f;
  c.eyescale = 0.f;
  ASSERT_NEAR(StereoView_HalfIpdInches(c), 0.f, 0.0001);

  c.eyescale = 1.f;
  const float half = StereoView_HalfIpdInches(c);
  ASSERT_NEAR(half, 0.064f * kInchesPerMeter * 0.5f, 0.001);

  c.eyescale = 0.5f;
  ASSERT_NEAR(StereoView_HalfIpdInches(c), half * 0.5f, 0.001);
}

TEST(stereo_mono_keeps_cyclopean_and_zero_pose) {
  StereoViewIn in;
  in.origin = {100.f, 200.f, 64.f};
  in.angles = {0.f, 45.f, 0.f};
  in.calib.ipd_m = 0.064f;
  in.calib.eyescale = 1.f;
  const auto p = StereoView_Decide(in, false);
  ASSERT_TRUE(p.valid);
  ASSERT_FALSE(p.painted_dual);
  ASSERT_STREQ(p.reason, "mono_identity_pose");
  ASSERT_NEAR(p.left.origin.x, in.origin.x, 0.0001);
  ASSERT_NEAR(p.left.origin.y, in.origin.y, 0.0001);
  ASSERT_NEAR(p.right.origin.z, in.origin.z, 0.0001);
  ASSERT_NEAR(p.left.pose_x, 0.f, 0.0001);
  ASSERT_NEAR(p.right.pose_x, 0.f, 0.0001);
  ASSERT_NEAR(StereoView_SubmitPoseX(in.calib, 0, false), 0.f, 0.0001);
  ASSERT_NEAR(StereoView_SubmitPoseX(in.calib, 1, false), 0.f, 0.0001);
}

TEST(stereo_dual_offsets_along_head_right) {
  StereoViewIn in;
  in.origin = {10.f, 20.f, 30.f};
  in.angles = {0.f, 0.f, 0.f}; // Source right = (0, -1, 0)
  in.fov = 90.f;
  in.aspect = 1.6f;
  in.calib.ipd_m = 0.064f;
  in.calib.eyescale = 1.f;
  const auto p = StereoView_Decide(in, true);
  ASSERT_TRUE(p.painted_dual);
  ASSERT_STREQ(p.reason, "dual_ipd_origin");
  ASSERT_NEAR(p.left.angles.y, in.angles.y, 0.0001);
  ASSERT_NEAR(p.right.angles.y, in.angles.y, 0.0001);
  ASSERT_NEAR(p.left.fov, p.right.fov, 0.0001);
  ASSERT_NEAR(p.left.aspect, p.right.aspect, 0.0001);

  const float half = p.half_ipd;
  ASSERT_NEAR(p.left.origin.x, 10.f, 0.001);
  ASSERT_NEAR(p.right.origin.x, 10.f, 0.001);
  ASSERT_NEAR(p.left.origin.y, 20.f + half, 0.001);  // +Y is left
  ASSERT_NEAR(p.right.origin.y, 20.f - half, 0.001); // -Y is right
  ASSERT_NEAR(p.left.origin.z, 30.f, 0.001);
  ASSERT_NEAR(p.right.origin.z, 30.f, 0.001);

  const Vec3 sep = p.right.origin - p.left.origin;
  Vec3 right;
  AngleVectors(in.angles, nullptr, &right, nullptr);
  ASSERT_NEAR(sep.x, right.x * (2.f * half), 0.001);
  ASSERT_NEAR(sep.y, right.y * (2.f * half), 0.001);
  ASSERT_NEAR(sep.z, right.z * (2.f * half), 0.001);
}

TEST(stereo_dual_yaw90_still_along_right) {
  StereoViewIn in;
  in.origin = {0.f, 0.f, 0.f};
  in.angles = {0.f, 90.f, 0.f};
  in.calib.ipd_m = 0.08f;
  in.calib.eyescale = 1.f;
  const auto p = StereoView_Decide(in, true);
  Vec3 right;
  AngleVectors(in.angles, nullptr, &right, nullptr);
  const Vec3 expect_l = in.origin + right * (-p.half_ipd);
  const Vec3 expect_r = in.origin + right * (p.half_ipd);
  ASSERT_NEAR(p.left.origin.x, expect_l.x, 0.001);
  ASSERT_NEAR(p.left.origin.y, expect_l.y, 0.001);
  ASSERT_NEAR(p.right.origin.x, expect_r.x, 0.001);
  ASSERT_NEAR(p.right.origin.y, expect_r.y, 0.001);
}

TEST(stereo_pose_x_never_on_lens_blit) {
  Calib c;
  c.ipd_m = 0.064f;
  c.eyescale = 1.f;
  ASSERT_NEAR(StereoView_SubmitPoseX(c, 0, true), 0.f, 0.0001);
  ASSERT_NEAR(StereoView_SubmitPoseX(c, 1, true), 0.f, 0.0001);
  c.swap_eyes = true;
  ASSERT_NEAR(StereoView_SubmitPoseX(c, 0, true), 0.f, 0.0001);
  ASSERT_NEAR(StereoView_SubmitPoseX(c, 1, false), 0.f, 0.0001);
}

TEST(stereo_dual_ipd_ignores_uv_eyescale) {
  Calib c;
  c.ipd_m = 0.064f;
  c.eyescale = 0.13f; // live Vision knob — UV only
  ASSERT_NEAR(StereoView_SubmitPoseX(c, 0, true), 0.f, 0.0001);
  ASSERT_NEAR(StereoView_SubmitPoseX(c, 1, true), 0.f, 0.0001);
  ASSERT_NEAR(StereoView_HalfIpdInches(c, kInchesPerMeter, true), 0.064f * kInchesPerMeter * 0.5f,
              0.001);
  ASSERT_TRUE(StereoView_HalfIpdInches(c, kInchesPerMeter, false) < 0.3f);

  StereoViewIn in;
  in.origin = {0.f, 0.f, 64.f};
  in.angles = {0.f, 0.f, 0.f};
  in.calib = c;
  const auto p = StereoView_Decide(in, true);
  const float full = 0.064f * kInchesPerMeter * 0.5f;
  ASSERT_NEAR(p.half_ipd, full, 0.001);
  ASSERT_NEAR(p.half_ipd_m, 0.032f, 0.0001);
  ASSERT_NEAR(p.left.pose_x, 0.f, 0.0001);
  ASSERT_NEAR(p.right.pose_x, 0.f, 0.0001);
  ASSERT_NEAR(p.left.origin.y - in.origin.y, full, 0.001);
  ASSERT_NEAR(p.right.origin.y - in.origin.y, -full, 0.001);
}

TEST(stereo_uv_crop_never_fakes_ipd) {
  Calib c;
  c.eyescale = 1.f;
  c.hoffset = 0.2f;
  const auto mono_l = CalibSubmitCrop(c, 0, false);
  const auto mono_r = CalibSubmitCrop(c, 1, false);
  ASSERT_NEAR(mono_l.u0, mono_r.u0, 0.0001);
  ASSERT_NEAR(mono_l.u1, mono_r.u1, 0.0001);
  const auto dual_l = CalibSubmitCrop(c, 0, true);
  const auto dual_r = CalibSubmitCrop(c, 1, true);
  ASSERT_NEAR(dual_l.u0, dual_r.u0, 0.0001);
  ASSERT_NEAR(dual_l.u1, dual_r.u1, 0.0001);
  ASSERT_NEAR(dual_l.u0, mono_l.u0, 0.0001);
  ASSERT_NEAR(dual_l.pose_x, 0.f, 0.0001);
  ASSERT_NEAR(dual_r.pose_x, 0.f, 0.0001);
  // Shared Vision pan stays; it is not a second IPD plane.
  Calib flat = c;
  flat.hoffset = 0.f;
  const auto centered = CalibSubmitCrop(flat, 0, true);
  ASSERT_TRUE(dual_l.u0 > centered.u0);
}
