#include "cssvrmod/tick.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(tick_fire_ak_and_knife_melee) {
  TickIn in;
  in.xr.hmd.valid = true;
  in.xr.hmd.ang = {0, 0, 0};
  in.xr.right.valid = true;
  in.xr.right.pos = {0, 0, 40};
  in.xr.right.ang = {0, 90, 0};
  in.xr.right.vel = {120, 0, 0};
  in.xr.trigger_r = 0.9f;
  in.wep = FindWeapon("weapon_ak47");
  in.now = 1.f;
  WallState L, R;
  float next = 0.f;
  auto fire = Tick(in, L, R, &next);
  ASSERT_TRUE(fire.gun.valid);
  ASSERT_TRUE(fire.aim.valid);
  ASSERT_TRUE(fire.cmd.firing);
  ASSERT_STREQ(fire.status, "firing");
  ASSERT_TRUE(fire.laser.path_ok);

  in.xr.trigger_r = 0.f;
  in.xr.trigger_l = 0.9f; // melee intent (off-hand)
  in.wep = FindWeapon("weapon_knife");
  in.world_melee_hit = true;
  auto knife = Tick(in, L, R, &next);
  ASSERT_TRUE(knife.melee.hit);
  ASSERT_TRUE(knife.melee.damage > 20.f);
  ASSERT_STREQ(knife.status, "melee_hit");
}

TEST(tick_panel_freezes_stick_turn) {
  TickIn in;
  in.xr.hmd.valid = true;
  in.xr.panel_visible = true;
  in.xr.stick_rx = 1.f;
  in.input.smooth_turn = true;
  in.input.snap_turn = false;
  in.input.turn_speed = 90.f;
  in.dt = 1.f;
  TurnState turn;
  turn.yaw_off = -15.f;
  in.turn = &turn;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_NEAR(turn.yaw_off, -15.f, 0.1f);
  ASSERT_NEAR(o.cmd.view_yaw, -15.f, 0.1f);
}

TEST(tick_panel_blocks_combat) {
  TickIn in;
  in.xr.hmd.valid = true;
  in.xr.panel_visible = true;
  in.xr.right.valid = true;
  in.xr.right.pos = {0, 0, 40};
  in.xr.right.ang = {0, 90, 0};
  in.xr.trigger_r = 0.9f;
  in.xr.trigger_l = 0.9f;
  in.wep = FindWeapon("weapon_knife");
  in.world_melee_hit = true;
  in.now = 1.f;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_FALSE(o.cmd.firing);
  ASSERT_FALSE(o.melee.hit);
  ASSERT_STREQ(o.laser.reason, "focus_primary");
}

TEST(tick_fist_melee_starts_at_knuckles) {
  // Wrist-only 5u reach misses a wall 6u ahead; Lua fist src is +5 along forward.
  // Point traces (gun tip) miss so this is the fist offset, not the barrel ray.
  auto world = [](Vec3 start, Vec3 end, Vec3 mins, Vec3 maxs) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    const bool hull = mins.LengthSqr() > 0.01f || maxs.LengthSqr() > 0.01f;
    if (!hull) return t;
    if (start.x < 6.f && end.x >= 6.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_normal = {-1, 0, 0};
      t.fraction = (6.f - start.x) / (end.x - start.x);
      t.hit_pos = {6.f, start.y, start.z};
    }
    return t;
  };
  TickIn in;
  in.xr.left.valid = true;
  in.xr.left.pos = {0, 0, 40};
  in.xr.left.ang = {0, 0, 0}; // +X — off-hand fist
  in.xr.left.vel = {80, 0, 0};
  in.xr.right.valid = true; // primary present but must not own the fist hull
  in.xr.right.pos = {100, 0, 40};
  in.xr.right.ang = {0, 0, 0};
  in.xr.right.vel = {80, 0, 0};
  in.xr.trigger_l = 0.9f; // melee intent (off-hand trigger)
  in.trace = world;
  in.now = 1.f;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_TRUE(o.melee.hit);
  ASSERT_EQ(static_cast<int>(o.melee.hand), static_cast<int>(Hand::Left));
  ASSERT_NEAR(o.melee.src.x, 5.f, 0.1);
  ASSERT_STREQ(o.status, "melee_hit");
}

TEST(tick_offhand_fist_ignores_primary_swing) {
  // Primary is at the wall; off-hand is not. Fist used to hull the gun hand.
  auto world = [](Vec3 start, Vec3 end, Vec3 mins, Vec3 maxs) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    const bool hull = mins.LengthSqr() > 0.01f || maxs.LengthSqr() > 0.01f;
    if (!hull) return t;
    if (start.x < 6.f && end.x >= 6.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_normal = {-1, 0, 0};
      t.fraction = (6.f - start.x) / (end.x - start.x);
      t.hit_pos = {6.f, start.y, start.z};
    }
    return t;
  };
  TickIn in;
  in.xr.right.valid = true;
  in.xr.right.pos = {0, 0, 40};
  in.xr.right.ang = {0, 0, 0};
  in.xr.right.vel = {80, 0, 0};
  in.xr.left.valid = true;
  in.xr.left.pos = {100, 0, 40};
  in.xr.left.ang = {0, 0, 0};
  in.xr.left.vel = {80, 0, 0};
  in.xr.trigger_l = 0.9f;
  in.trace = world;
  in.now = 1.f;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_FALSE(o.melee.hit);
  ASSERT_EQ(static_cast<int>(o.melee.hand), static_cast<int>(Hand::Left));
  ASSERT_NEAR(o.melee.src.x, 105.f, 0.1);
}

TEST(tick_melee_ignores_hmd_walk) {
  // World 80 u/s on both HMD and fist — Lua relative vel is 0; used to punch.
  TickIn in;
  in.xr.hmd.valid = true;
  in.xr.hmd.vel = {80, 0, 0};
  in.xr.left.valid = true;
  in.xr.left.pos = {0, 0, 40};
  in.xr.left.ang = {0, 0, 0};
  in.xr.left.vel = {80, 0, 0};
  in.xr.trigger_l = 0.9f;
  in.world_melee_hit = true;
  in.now = 1.f;
  WallState L, R;
  float next = 0.f;
  auto walk = Tick(in, L, R, &next);
  ASSERT_FALSE(walk.melee.hit);
  ASSERT_STREQ(walk.melee.reason, "below_threshold");

  in.xr.left.vel = {160, 0, 0}; // 80 relative
  auto punch = Tick(in, L, R, &next);
  ASSERT_TRUE(punch.melee.hit);
  ASSERT_STREQ(punch.status, "melee_hit");
}

TEST(tick_knife_sweep_uses_trace) {
  auto world = [](Vec3 start, Vec3 end, Vec3, Vec3) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    if (end.y > 10.f) {
      t.hit = true;
      t.fraction = 0.3f;
      t.hit_world = true;
      t.hit_pos = {start.x, 10.f, start.z};
      t.hit_normal = {0, -1, 0};
    }
    return t;
  };
  TickIn in;
  in.xr.right.valid = true;
  in.xr.right.pos = {0, 0, 40};
  in.xr.right.ang = {0, 90, 0}; // +Y
  in.xr.right.vel = {80, 0, 0};
  in.xr.trigger_l = 0.9f;
  in.wep = FindWeapon("weapon_knife");
  in.trace = world;
  in.now = 1.f;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_TRUE(o.melee.hit);
  ASSERT_STREQ(o.status, "melee_hit");
}

TEST(tick_left_handed_gun_slaves_primary) {
  TickIn in;
  in.input.left_handed = true;
  in.xr.hmd.valid = true;
  in.xr.hmd.ang = {0, 0, 0};
  in.xr.left.valid = true;
  in.xr.left.pos = {10, 20, 40};
  in.xr.left.ang = {0, 90, 0}; // +Y
  in.xr.right.valid = true;
  in.xr.right.pos = {0, 0, 40};
  in.xr.right.ang = {0, 0, 0}; // +X — must not own the gun
  in.xr.trigger_l = 0.9f;
  in.wep = FindWeapon("weapon_ak47");
  WallState L, R;
  float next = 0.f;
  auto fire = Tick(in, L, R, &next);
  ASSERT_TRUE(fire.gun.valid);
  ASSERT_TRUE(fire.aim.valid);
  ASSERT_TRUE(fire.cmd.firing);
  ASSERT_STREQ(fire.laser.primary_hand, "left");
  ASSERT_NEAR(fire.aim.dir.y, 1.f, 0.05);
  ASSERT_TRUE(std::fabs(fire.aim.dir.x) < 0.2f);
  ASSERT_TRUE(fire.gun.pos.DistToSqr({10, 20, 40}) < fire.gun.pos.DistToSqr({0, 0, 40}));

  in.xr.trigger_l = 0.f;
  in.xr.trigger_r = 0.9f; // off-hand melee
  in.wep = FindWeapon("weapon_knife");
  in.world_melee_hit = true;
  in.xr.left.vel = {120, 0, 0};
  in.now = 1.f;
  auto knife = Tick(in, L, R, &next);
  ASSERT_TRUE(knife.melee.hit);
  ASSERT_EQ(static_cast<int>(knife.melee.hand), static_cast<int>(Hand::Left));
  ASSERT_NEAR(knife.melee.src.x, 10.f, 0.1);
}

TEST(tick_wall_blocks_hand) {
  auto world = [](Vec3 start, Vec3 end, Vec3, Vec3) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    auto solid = [](const Vec3& p) { return p.x >= 50.f; };
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
    if (start.x < 50.f && end.x >= 50.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_pos = {50, start.y, start.z};
      t.hit_normal = {-1, 0, 0};
      t.fraction = (50.f - start.x) / (end.x - start.x);
    }
    return t;
  };
  TickIn in;
  in.xr.right.valid = true;
  in.xr.right.pos = {80, 0, 40};
  in.xr.right.ang = {0, 0, 0};
  in.trace = world;
  WallState L, R;
  R.last_free = {10, 0, 40};
  R.has_free = true;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_TRUE(o.right_wall.clipped);
  ASSERT_TRUE(o.right_resolved.pos.x < 80.f);
}

TEST(tick_wall_lock_releases_on_hmd_teleport) {
  auto world = [](Vec3 start, Vec3 end, Vec3, Vec3) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    auto solid = [](const Vec3& p) { return p.x >= 50.f; };
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
    if (start.x < 50.f && end.x >= 50.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_pos = {50, start.y, start.z};
      t.hit_normal = {-1, 0, 0};
      t.fraction = (50.f - start.x) / (end.x - start.x);
    }
    return t;
  };
  TickIn in;
  in.xr.right.valid = true;
  in.xr.right.pos = {80, 0, 40};
  in.xr.right.ang = {0, 0, 0};
  in.trace = world;
  WallState L, R;
  R.last_free = {10, 0, 40};
  R.has_free = true;
  float next = 0.f;

  // Nearby HMD: keep last-free so the hand does not punch through.
  in.xr.hmd.valid = true;
  in.xr.hmd.pos = {20, 0, 40};
  auto keep = Tick(in, L, R, &next);
  ASSERT_TRUE(keep.right_wall.clipped);
  ASSERT_TRUE(keep.right_resolved.pos.x < 50.f);

  // Spawn/teleport: last-free is a map away from the HMD — do not yank the hand.
  R.last_free = {10, 0, 40};
  R.has_free = true;
  in.xr.hmd.pos = {200, 0, 40};
  auto drop = Tick(in, L, R, &next);
  ASSERT_FALSE(drop.right_wall.clipped);
  ASSERT_STREQ(drop.right_wall.reason, "hmd_teleport");
  ASSERT_TRUE(drop.right_resolved.pos.x > 40.f);
  ASSERT_NEAR(drop.right_resolved.pos.x, 80.f, 0.1);
}

TEST(tick_wall_correction_caps_last_free_yank) {
  // Inward hit normal → rest is solid → last-free snap. Nearby HMD so teleport
  // release does not fire; clamp must still stop a 70u yank.
  auto world = [](Vec3 start, Vec3 end, Vec3, Vec3) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    auto solid = [](const Vec3& p) { return p.x >= 50.f; };
    t.start_solid = solid(start);
    t.all_solid = solid(start) && solid(end);
    if (t.start_solid) {
      t.hit = true;
      t.hit_world = true;
      t.hit_pos = start;
      t.hit_normal = {1, 0, 0};
      t.fraction = 0.f;
      return t;
    }
    if (start.x < 50.f && end.x >= 50.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_pos = {50, start.y, start.z};
      t.hit_normal = {1, 0, 0};
      t.fraction = (50.f - start.x) / (end.x - start.x);
    }
    return t;
  };
  TickIn in;
  in.xr.hmd.valid = true;
  in.xr.hmd.pos = {20, 0, 40};
  in.xr.right.valid = true;
  in.xr.right.pos = {80, 0, 40};
  in.xr.right.ang = {0, 90, 0}; // +Y — tip must not also pull off the X wall
  in.trace = world;
  WallState L, R;
  R.last_free = {10, 0, 40};
  R.has_free = true;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_TRUE(o.right_wall.clipped);
  ASSERT_TRUE(o.right_resolved.pos.x > 20.f); // not snapped to last-free
  ASSERT_TRUE(o.right_resolved.pos.x < 80.f);
  ASSERT_NEAR(o.right_resolved.pos.x, 40.f, 1.f);
}

TEST(tick_offhand_hull_blocks_before_wrist) {
  // Wrist is free; knuckles 2.5u ahead are in the wall. Point traces (tip) miss
  // so this is the off-hand sphere offset, not the gun box / barrel ray.
  auto world = [](Vec3 start, Vec3 end, Vec3 mins, Vec3 maxs) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    const bool hull = mins.LengthSqr() > 0.01f || maxs.LengthSqr() > 0.01f;
    if (!hull) return t;
    auto solid = [](const Vec3& p) { return p.x >= 22.f; };
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
    if (start.x < 22.f && end.x >= 22.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_pos = {22, start.y, start.z};
      t.hit_normal = {-1, 0, 0};
      t.fraction = (22.f - start.x) / (end.x - start.x);
    }
    return t;
  };
  TickIn in;
  in.xr.left.valid = true;
  in.xr.left.pos = {20, 0, 40};
  in.xr.left.ang = {0, 0, 0}; // +X, knuckle sample ~2.5u ahead
  in.trace = world;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_TRUE(o.left_wall.clipped);
  ASSERT_TRUE(o.left_resolved.pos.x < 20.f);
}

TEST(tick_gun_hull_blocks_before_wrist) {
  // Wrist is free; the 10u gun sample is in the wall. Point traces (tip) miss
  // so this is the hull offset, not the barrel ray.
  auto world = [](Vec3 start, Vec3 end, Vec3 mins, Vec3 maxs) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    const bool hull = mins.LengthSqr() > 0.01f || maxs.LengthSqr() > 0.01f;
    if (!hull) return t;
    auto solid = [](const Vec3& p) { return p.x >= 28.f; };
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
    if (start.x < 28.f && end.x >= 28.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_pos = {28, start.y, start.z};
      t.hit_normal = {-1, 0, 0};
      t.fraction = (28.f - start.x) / (end.x - start.x);
    }
    return t;
  };
  TickIn in;
  in.xr.right.valid = true;
  in.xr.right.pos = {20, 0, 40};
  in.xr.right.ang = {0, 0, 0}; // +X, gun sample ~10u ahead
  in.wep = FindWeapon("weapon_ak47");
  in.trace = world;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_TRUE(o.right_wall.clipped);
  ASSERT_TRUE(o.right_resolved.pos.x < 20.f);
}

TEST(tick_empty_primary_is_not_a_gun) {
  // Live hook has no weapon query. Gun 10u hull / 18u barrel used to yank an empty fist.
  auto world = [](Vec3 start, Vec3 end, Vec3 mins, Vec3 maxs) {
    TraceHit t;
    t.start_pos = start;
    t.end_pos = end;
    const bool hull = mins.LengthSqr() > 0.01f || maxs.LengthSqr() > 0.01f;
    auto solid = [](const Vec3& p) { return p.x >= 28.f; };
    if (hull) {
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
      if (start.x < 28.f && end.x >= 28.f) {
        t.hit = true;
        t.hit_world = true;
        t.hit_pos = {28, start.y, start.z};
        t.hit_normal = {-1, 0, 0};
        t.fraction = (end.x > start.x) ? (28.f - start.x) / (end.x - start.x) : 0.f;
      }
      return t;
    }
    if (start.x < 32.f && end.x >= 32.f) {
      t.hit = true;
      t.hit_world = true;
      t.hit_normal = {-1, 0, 0};
      t.fraction = (32.f - start.x) / (end.x - start.x);
      t.hit_pos = {32.f, start.y, start.z};
    }
    return t;
  };
  TickIn in;
  in.xr.right.valid = true;
  in.xr.right.pos = {20, 0, 40};
  in.xr.right.ang = {0, 0, 0};
  in.trace = world;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_FALSE(o.right_wall.clipped);
  ASSERT_FALSE(o.gun.valid);
  ASSERT_NEAR(o.right_resolved.pos.x, 20.f, 0.1);
}

TEST(tick_empty_fire_keeps_hmd_look) {
  // Fist +Y would snap view to 90 if GunFromHand ran with the default offset.
  TickIn in;
  in.xr.hmd.valid = true;
  in.xr.hmd.ang = {0, 0, 0};
  in.xr.right.valid = true;
  in.xr.right.pos = {0, 0, 40};
  in.xr.right.ang = {0, 90, 0};
  in.xr.trigger_r = 0.9f;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_TRUE(o.cmd.firing);
  ASSERT_FALSE(o.gun.valid);
  ASSERT_FALSE(o.cmd.look_from_gun);
  ASSERT_NEAR(o.cmd.view_yaw, 0.f, 2.f);
  ASSERT_STREQ(o.cmd.reason, "aim_hmd");
}

TEST(tick_weapon_tip_blocks_muzzle) {
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
  TickIn in;
  in.xr.right.valid = true;
  in.xr.right.pos = {20, 0, 40};
  in.xr.right.ang = {0, 0, 0}; // +X, barrel toward the wall
  in.wep = FindWeapon("weapon_ak47");
  in.trace = world;
  WallState L, R;
  float next = 0.f;
  auto o = Tick(in, L, R, &next);
  ASSERT_TRUE(o.gun.valid);
  ASSERT_TRUE(o.right_wall.clipped);
  ASSERT_TRUE(o.right_resolved.pos.x < 20.f); // wrist pulled off the barrel-in-wall
}
