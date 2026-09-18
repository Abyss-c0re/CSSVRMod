#include "cssvrmod/stereo_view.hpp"
#include "cssvrmod/vk_eye.hpp"
#include "test_framework.h"
#include <vector>

using namespace cssvr;

TEST(vk_eye_store_requires_two_distinct) {
  VkEyePair p;
  ASSERT_FALSE(VkEye_Ready(p));
  std::vector<unsigned char> a(16, 1), b(16, 2);
  ASSERT_TRUE(VkEye_Store(&p, 0, a.data(), 2, 2, false));
  ASSERT_FALSE(VkEye_Ready(p));
  ASSERT_TRUE(VkEye_Store(&p, 1, b.data(), 2, 2, false));
  ASSERT_TRUE(VkEye_Ready(p));
  ASSERT_TRUE(p.eye[0].px.data() != p.eye[1].px.data());
}

TEST(vk_eye_reject_mismatch_and_tiny) {
  VkEyePair p;
  std::vector<unsigned char> a(16, 3), b(64, 4);
  ASSERT_TRUE(VkEye_Store(&p, 0, a.data(), 2, 2, true));
  ASSERT_TRUE(VkEye_Store(&p, 1, b.data(), 4, 4, true));
  ASSERT_FALSE(VkEye_Ready(p));
  ASSERT_FALSE(VkEye_Store(&p, 0, a.data(), 1, 1, false));
  VkEye_Clear(&p);
  ASSERT_FALSE(VkEye_Ready(p));
}

TEST(vk_eye_ready_does_not_unlock_pose_ipd_alone) {
  // Storing two CPU frames is not a world paint. Pose IPD still needs painted_dual.
  Calib c;
  c.ipd_m = 0.064f;
  c.eyescale = 1.f;
  VkEyePair p;
  std::vector<unsigned char> a(16, 9), b(16, 8);
  VkEye_Store(&p, 0, a.data(), 2, 2, false);
  VkEye_Store(&p, 1, b.data(), 2, 2, false);
  ASSERT_TRUE(VkEye_Ready(p));
  ASSERT_NEAR(StereoView_SubmitPoseX(c, 0, false), 0.f, 0.0001);
  ASSERT_NEAR(StereoView_SubmitPoseX(c, 0, true), -0.032f, 0.0001);
}
