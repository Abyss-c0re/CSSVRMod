#pragma once
// One CSSVR frame: poses → wall resolve → gun/aim → input → melee.
#include "aim.hpp"
#include "collision.hpp"
#include "hand_bullet.hpp"
#include "input.hpp"
#include "melee.hpp"
#include "weapons.hpp"

namespace cssvr {

struct TickIn {
  XrSample xr;
  const WeaponInfo* wep = nullptr;
  float now = 0.f;
  float dt = 0.011f;
  InputConfig input;
  MeleeConfig melee;
  float wall_pad = 0.75f;
  float hand_radius = kDefaultRadius;
  bool world_melee_hit = false; // mock when no trace
  Ang3 current_view;
  float mouse_sens = 0.022f;
  TraceFn trace; // empty → skip wall / melee sweep
  HandVelState* hand_vel = nullptr; // gun / melee hand (primary)
  TurnState* turn = nullptr;        // stick locomotion yaw (persists across ticks)
};

struct TickOut {
  Pose left_resolved;
  Pose right_resolved;
  GunPose gun;
  AimRay aim;
  LaserDecision laser;
  UserCmdOverlay cmd;
  SdlInjectPlan sdl;
  MeleeDecision melee;
  WallResolve left_wall;
  WallResolve right_wall;
  const char* status = "ok";
};

inline TickOut Tick(TickIn in, WallState& leftWall, WallState& rightWall, float* next_melee) {
  TickOut o;
  o.left_resolved = in.xr.left;
  o.right_resolved = in.xr.right;

  if (in.trace) {
    if (in.xr.left.valid) {
      o.left_wall =
          ResolveHandWallSweep(in.xr.left.pos, leftWall, in.hand_radius, in.wall_pad, in.trace);
      o.left_wall = ApplyWallLockRelease(o.left_wall, leftWall, in.xr.left.pos, in.xr.hmd.valid,
                                        in.xr.hmd.pos);
      o.left_resolved.pos = ApplyHandCorrection(in.xr.left.pos, o.left_wall.pos);
      if (!o.left_wall.clipped) {
        leftWall.last_free = o.left_resolved.pos;
        leftWall.has_free = true;
      }
    }
    if (in.xr.right.valid) {
      o.right_wall =
          ResolveHandWallSweep(in.xr.right.pos, rightWall, in.hand_radius, in.wall_pad, in.trace);
      o.right_wall = ApplyWallLockRelease(o.right_wall, rightWall, in.xr.right.pos, in.xr.hmd.valid,
                                         in.xr.hmd.pos);
      o.right_resolved.pos = ApplyHandCorrection(in.xr.right.pos, o.right_wall.pos);
      if (!o.right_wall.clipped) {
        rightWall.last_free = o.right_resolved.pos;
        rightWall.has_free = true;
      }
    }
  }

  WeaponOffset off;
  if (in.wep) off = in.wep->offset;
  if (in.wep && in.wep->muzzle_len > 0.f) off.muzzle_len = in.wep->muzzle_len;
  // Gun slaves the primary hand (aim.hpp). Left-handed must not keep the AK on the right.
  Pose primary = in.input.left_handed ? o.left_resolved : o.right_resolved;
  if (in.trace && primary.valid) {
    const float tip_len = off.muzzle_len > 0.f ? off.muzzle_len : 18.f;
    const WeaponTipResolve tip =
        ApplyWeaponTip(primary.pos, primary.ang, tip_len, in.wall_pad, in.trace);
    if (tip.clipped) {
      primary.pos = tip.hand_pos;
      if (in.input.left_handed) {
        o.left_resolved.pos = primary.pos;
        o.left_wall.clipped = true;
      } else {
        o.right_resolved.pos = primary.pos;
        o.right_wall.clipped = true;
      }
    }
  }
  o.gun = GunFromHand(primary, off);
  o.aim = ResolveMuzzle(o.gun, primary);

  LaserOpts lo;
  lo.vr_active = in.xr.hmd.valid;
  lo.laser_on = true;
  lo.has_primary_pose = primary.valid;
  lo.primary_hand = in.input.left_handed ? "left" : "right";
  lo.laser_hand = lo.primary_hand;
  lo.menu_focus = in.xr.panel_visible;
  o.laser = Laser_Decide(lo);

  o.cmd = InputMap(in.xr, o.gun, in.input, in.dt, in.turn);
  o.sdl = PlanSdlInject(o.cmd, in.current_view, in.mouse_sens);

  MeleeSample ms;
  const bool knife = in.wep && in.wep->is_melee;
  if (!in.xr.panel_visible && (knife || o.cmd.melee_intent)) {
    const Pose& hand = primary;
    ms.pos = hand.pos;
    ms.dir = Forward(hand.ang);
    ms.vel = HandVelOrDelta(hand, in.now, in.hand_vel);
    ms.hand = in.input.left_handed ? Hand::Left : Hand::Right;
    ms.impact = knife ? in.wep->melee_impact : ImpactType::Fist;
    ms.use_weapon = knife;
    ms.is_melee_weapon = knife;
    if (knife) ms.weapon_base_damage = in.wep->damage;
    ms.reach = knife ? 24.f : kDefaultReach;
    bool world_hit = in.world_melee_hit;
    if (in.trace) world_hit = MeleeSweepHit(ms, in.trace);
    o.melee = MeleeDecide(ms, world_hit, in.melee, in.now, next_melee);
  }

  if (o.melee.hit) o.status = "melee_hit";
  else if (o.cmd.firing) o.status = "firing";
  else if (o.right_wall.clipped || o.left_wall.clipped) o.status = "wall";
  else o.status = "ok";
  return o;
}

} // namespace cssvr
