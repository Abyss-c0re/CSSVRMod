#include "cssvrmod/engine_trace.hpp"
#include "cssvrmod/source_if.hpp"
#include "test_framework.h"
#include <cstddef>
#include <cstring>

using namespace cssvr;

TEST(ray_init_hull_swept) {
  RayBlob r;
  Ray_InitHull(&r, {0, 0, 0}, {10, 0, 0}, {-2, -2, -2}, {2, 2, 2});
  ASSERT_TRUE(r.is_swept);
  ASSERT_FALSE(r.is_ray);
  ASSERT_NEAR(r.delta.x, 10.f, 0.001);
  ASSERT_NEAR(r.extents.x, 2.f, 0.001);
  ASSERT_NEAR(r.start.x, 0.f, 0.001);
}

TEST(ray_blob_2013_world_axis_at_64) {
  ASSERT_EQ((int)offsetof(RayBlob, world_axis), 64);
  ASSERT_EQ((int)offsetof(RayBlob, is_ray), 72);
}

TEST(ray_init_point_not_swept) {
  RayBlob r;
  Ray_InitHull(&r, {1, 2, 3}, {1, 2, 3}, {0, 0, 0}, {0, 0, 0});
  ASSERT_FALSE(r.is_swept);
  ASSERT_TRUE(r.is_ray);
}

TEST(trace_blob_parse_hit) {
  unsigned char blob[64] = {};
  Vec3 end{4.f, 5.f, 6.f};
  Vec3 n{0.f, 1.f, 0.f};
  float frac = 0.4f;
  std::memcpy(blob + kTrEnd, &end, 12);
  std::memcpy(blob + kTrNormal, &n, 12);
  std::memcpy(blob + kTrFraction, &frac, 4);
  blob[kTrStartSolid] = 0;
  blob[kTrAllSolid] = 0;
  auto h = TraceHitFromBlob(blob, sizeof(blob));
  ASSERT_TRUE(h.hit);
  ASSERT_NEAR(h.fraction, 0.4f, 0.001);
  ASSERT_NEAR(h.hit_normal.y, 1.f, 0.001);
  ASSERT_NEAR(h.end_pos.z, 6.f, 0.001);
}

TEST(trace_blob_insane_fraction_is_miss) {
  unsigned char blob[64] = {};
  float frac = 99.f;
  std::memcpy(blob + kTrFraction, &frac, 4);
  auto h = TraceHitFromBlob(blob, sizeof(blob));
  ASSERT_FALSE(h.hit);
  ASSERT_NEAR(h.fraction, 1.f, 0.001);
}

TEST(engine_make_trace_empty_without_selftest) {
  EngineIf e;
  auto fn = EngineMakeTraceFn(e);
  ASSERT_FALSE((bool)fn);
}

TEST(engine_trace_miss_toast) {
  EngineTraceToastIn in;
  ASSERT_FALSE(EngineTrace_ToastDecide(in).should_toast);
  in.probed = true;
  in.have_iface = false;
  auto none = EngineTrace_ToastDecide(in);
  ASSERT_TRUE(none.should_toast);
  ASSERT_FALSE(none.abort_vr);
  ASSERT_STREQ(none.reason, "no_trace");
  ASSERT_STREQ(none.label, "TR · MISS");
  ASSERT_TRUE(std::strstr(none.copy, "hulls") != nullptr);
  in.have_iface = true;
  auto fail = EngineTrace_ToastDecide(in);
  ASSERT_TRUE(fail.should_toast);
  ASSERT_STREQ(fail.reason, "selftest_fail");
  in.already_shown = true;
  ASSERT_FALSE(EngineTrace_ToastDecide(in).should_toast);
  in.already_shown = false;
  in.selftest_ok = true;
  auto ok = EngineTrace_ToastDecide(in);
  ASSERT_FALSE(ok.should_toast);
  ASSERT_STREQ(ok.reason, "trace_ok");
}
