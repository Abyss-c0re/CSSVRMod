#include "cssvrmod/melee.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(melee_hand_origin_is_knuckles) {
  const Vec3 fist = MeleeHandOrigin({0, 0, 40}, {0, 0, 0}); // +X
  ASSERT_NEAR(fist.x, 5.f, 0.05);
  ASSERT_NEAR(fist.z, 40.f, 0.05);
  const Vec3 left = MeleeHandOrigin({0, 0, 0}, {0, 90, 0}); // +Y
  ASSERT_NEAR(left.y, 5.f, 0.05);
}

TEST(melee_threshold_and_damage) {
  MeleeConfig c;
  ASSERT_NEAR(MeleeThresholdUnits(c), 75.f, 1e-4);
  ASSERT_NEAR(MeleeHeadThresholdUnits(c), 37.5f, 1e-4);
  ASSERT_NEAR(MeleeImpactMultiplier(ImpactType::Sharp), 1.5f, 1e-6);
  ASSERT_EQ(MeleeDamageType(ImpactType::Sharp), kDmgSlash);
  ASSERT_EQ(MeleeDamageType(ImpactType::Fist), kDmgClub);
  const float dmg = MeleeDamageAmount(20.f, 100.f, 0.05f, 1.5f);
  // speedFactor = min(5, 1+5) = 5; dmg = 20*5*1.5 = 150
  ASSERT_NEAR(dmg, 150.f, 1e-3);
  ASSERT_NEAR(MeleeSpeedFactor(0.f, 0.05f), 1.f, 1e-6);
}

TEST(melee_decide_gates) {
  MeleeConfig c;
  MeleeSample s;
  s.pos = {0, 0, 40};
  s.dir = {1, 0, 0};
  s.vel = {10, 0, 0}; // below 75
  s.hand = Hand::Right;
  s.impact = ImpactType::Sharp;
  s.weapon_base_damage = 20.f;
  float next = 0.f;
  auto hold = MeleeDecide(s, true, c, 1.f, &next);
  ASSERT_FALSE(hold.hit);
  ASSERT_STREQ(hold.reason, "below_threshold");
  ASSERT_STREQ(MeleeStatusLabel(hold), "MELEE · HOLD");

  s.vel = {80, 0, 0};
  auto miss = MeleeDecide(s, false, c, 1.f, &next);
  ASSERT_TRUE(miss.swing);
  ASSERT_FALSE(miss.hit);
  ASSERT_STREQ(MeleeStatusLabel(miss), "MELEE · SWING");

  auto hit = MeleeDecide(s, true, c, 1.f, &next);
  ASSERT_TRUE(hit.hit);
  ASSERT_TRUE(hit.damage > 20.f);
  ASSERT_TRUE(next > 1.f);
  ASSERT_STREQ(MeleeStatusLabel(hit), "MELEE · HIT");

  auto cd = MeleeDecide(s, true, c, 1.1f, &next);
  ASSERT_FALSE(cd.hit);
  ASSERT_STREQ(cd.reason, "cooldown");
}

TEST(melee_sweep_and_vel_delta) {
  auto world = [](Vec3 start, Vec3 end, Vec3, Vec3) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    if (end.x > 20.f && start.x < 20.f) {
      t.hit = true;
      t.hit_world = true;
      t.fraction = 0.4f;
      t.hit_pos = {20, start.y, start.z};
      t.hit_normal = {-1, 0, 0};
    }
    return t;
  };
  MeleeSample s;
  s.pos = {0, 0, 40};
  s.dir = {1, 0, 0};
  s.reach = 30.f;
  ASSERT_TRUE(MeleeSweepHit(s, world));
  s.pos = {40, 0, 40}; // start past the wall
  ASSERT_FALSE(MeleeSweepHit(s, world));

  Pose p;
  p.pos = {0, 0, 0};
  p.vel = {};
  HandVelState st;
  auto v0 = HandVelOrDelta(p, 0.f, &st);
  ASSERT_NEAR(v0.Length(), 0.f, 0.001);
  p.pos = {8, 0, 0};
  auto v1 = HandVelOrDelta(p, 0.1f, &st);
  ASSERT_NEAR(v1.x, 80.f, 0.1);
}

TEST(melee_hand_vel_resets_on_stop) {
  Pose p;
  p.pos = {0, 0, 0};
  HandVelState st;
  HandVelOrDelta(p, 0.f, &st);
  p.pos = {80, 0, 0};
  auto swing = HandVelOrDelta(p, 0.1f, &st);
  ASSERT_TRUE(swing.Length() > 70.f);
  HandVel_Reset(&st);
  p.pos = {0, 0, 40}; // new session — must not inherit the old sample
  auto first = HandVelOrDelta(p, 1.f, &st);
  ASSERT_NEAR(first.Length(), 0.f, 0.001);
  HandVel_Reset(nullptr);
}
