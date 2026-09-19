#include "cssvrmod/collision.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(collision_boxes_and_melee_detect) {
  auto b = BoxesFromAABB({-10, -2, -2}, {10, 2, 2});
  ASSERT_TRUE(b.ex > 9.f);
  ASSERT_TRUE(b.horizontal.maxs.x > 0.f);
  ASSERT_TRUE(DetectMeleeFromAABB("models/weapons/w_knife_t.mdl", {-1, -1, -20}, {1, 1, 20}));
  ASSERT_FALSE(DetectMeleeFromAABB("models/weapons/w_ak47.mdl", {-20, -4, -4}, {20, 4, 4}));
  ASSERT_TRUE(ModelNameLooksMelee("weapon_knife"));
}

TEST(collision_last_free_wall) {
  // Wall at x=100, solid x>=100.
  auto world = [](Vec3 start, Vec3 end, Vec3, Vec3) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    auto solid = [](const Vec3& p) { return p.x >= 100.f; };
    t.start_solid = solid(start);
    t.all_solid = solid(start) && solid(end);
    if (t.start_solid) {
      t.hit = true;
      t.hit_world = true;
      t.hit_pos = start;
      t.hit_normal = {-1, 0, 0};
      t.fraction = 0.f;
      return t;
    }
    if (start.x < 100.f && end.x >= 100.f) {
      t.hit = true;
      t.hit_world = true;
      const float f = (100.f - start.x) / (end.x - start.x);
      t.fraction = f;
      t.hit_pos = {100.f, start.y, start.z};
      t.hit_normal = {-1, 0, 0};
    }
    return t;
  };

  WallState st;
  auto free = ResolveHandWallSweep({50, 0, 40}, st, 2.2f, 0.75f, world);
  ASSERT_FALSE(free.clipped);
  st.last_free = {50, 0, 40};
  st.has_free = true;

  auto blocked = ResolveHandWallSweep({140, 0, 40}, st, 2.2f, 0.75f, world);
  ASSERT_TRUE(blocked.clipped);
  ASSERT_TRUE(blocked.pos.x < 100.f + 1.f);
  ASSERT_STREQ(blocked.reason, "wall_rest");

  // Desired still solid, keep last free if rest fails — start_solid from last free path.
  auto buried = ResolveHandWallSweep({200, 0, 40}, st, 2.2f, 0.75f, world);
  ASSERT_TRUE(buried.clipped);
}

TEST(collision_floor_does_not_lock) {
  auto floor = [](Vec3 start, Vec3 end, Vec3, Vec3) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    if (end.z < 0.f && start.z >= 0.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_pos = {end.x, end.y, 0.f};
      t.hit_normal = {0, 0, 1};
      t.fraction = 0.5f;
    }
    return t;
  };
  WallState st;
  auto r = ResolveHandWallSweep({10, 10, 20}, st, 2.2f, 0.75f, floor);
  ASSERT_FALSE(r.clipped);
}

TEST(collision_weapon_tip_pulls_hand) {
  auto world = [](Vec3 start, Vec3 end, Vec3, Vec3) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    if (start.x < 32.f && end.x >= 32.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_normal = {-1, 0, 0};
      t.fraction = (32.f - start.x) / (end.x - start.x);
      t.hit_pos = {32.f, start.y, start.z};
    }
    return t;
  };
  auto r = ApplyWeaponTip({20, 0, 40}, Ang3{0, 0, 0}, 18.f, 0.75f, world);
  ASSERT_TRUE(r.clipped);
  ASSERT_TRUE(r.hand_pos.x < 20.f); // pulled back off the barrel-in-wall
}

TEST(collision_adjust_box_offsets_gun_forward) {
  const Vec3 gun = AdjustCollisionsBox({0, 0, 0}, {0, 0, 0}, false);
  ASSERT_NEAR(gun.x, 10.f, 0.1);
  ASSERT_NEAR(gun.z, 4.f, 0.1);
  const Vec3 knife = AdjustCollisionsBox({0, 0, 0}, {0, 0, 0}, true);
  ASSERT_NEAR(knife.x, 3.f, 0.1);
  const Vec3 wrist = WristFromHullSample({20, 0, 40}, {30, 0, 44}, {28, 0, 44});
  ASSERT_NEAR(wrist.x, 18.f, 0.1);
}

TEST(collision_offhand_sample_is_knuckles) {
  const Vec3 s = HandCollisionSample({20, 0, 40}, {0, 0, 0}); // +X
  ASSERT_NEAR(s.x, 22.5f, 0.05);
  ASSERT_NEAR(s.y, 0.f, 0.05);
  ASSERT_NEAR(s.z, 40.f, 0.05);
  const Vec3 left = HandCollisionSample({0, 0, 0}, {0, 90, 0}); // +Y
  ASSERT_NEAR(left.y, 2.5f, 0.05);
}

TEST(collision_wall_resets_on_stop) {
  WallState st;
  st.last_free = {50, 0, 40};
  st.has_free = true;
  Wall_Reset(&st);
  ASSERT_FALSE(st.has_free);
  ASSERT_NEAR(st.last_free.Length(), 0.f, 0.001);
  Wall_Reset(nullptr);
}

TEST(collision_hand_correction_clamps_and_deadzone) {
  const Vec3 desired{80, 0, 40};
  ASSERT_NEAR(ApplyHandCorrection(desired, {80.1f, 0, 40}).x, 80.f, 0.01); // dead-zone
  ASSERT_NEAR(ApplyHandCorrection(desired, {70, 0, 40}).x, 70.f, 0.01);    // under max
  const Vec3 yanked = ApplyHandCorrection(desired, {10, 0, 40});
  ASSERT_NEAR(yanked.x, 40.f, 0.2); // 70u last-free snap capped at 40
  ASSERT_TRUE(std::fabs(yanked.x - 10.f) > 20.f);
}

TEST(collision_wall_lock_releases_far_from_hmd) {
  ASSERT_FALSE(ShouldReleaseWallLock({10, 0, 40}, {20, 0, 40}));
  ASSERT_TRUE(ShouldReleaseWallLock({10, 0, 40}, {200, 0, 40}));

  WallState st;
  st.last_free = {10, 0, 40};
  st.has_free = true;
  WallResolve r;
  r.pos = {10, 0, 40};
  r.clipped = true;
  r.reason = "wall_rest";
  auto keep = ApplyWallLockRelease(r, st, {80, 0, 40}, true, {20, 0, 40});
  ASSERT_TRUE(keep.clipped);
  ASSERT_TRUE(st.has_free);
  ASSERT_STREQ(keep.reason, "wall_rest");

  auto drop = ApplyWallLockRelease(r, st, {80, 0, 40}, true, {200, 0, 40});
  ASSERT_FALSE(drop.clipped);
  ASSERT_FALSE(st.has_free);
  ASSERT_NEAR(drop.pos.x, 80.f, 0.01);
  ASSERT_STREQ(drop.reason, "hmd_teleport");

  st.has_free = true;
  auto no_hmd = ApplyWallLockRelease(r, st, {80, 0, 40}, false, {200, 0, 40});
  ASSERT_TRUE(no_hmd.clipped);
  ASSERT_TRUE(st.has_free);
}
