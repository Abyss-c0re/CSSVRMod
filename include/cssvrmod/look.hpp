#pragma once
// HMD drives look. Snap-on-fire may override. Invalid HMD keeps the game view.
#include "aim.hpp"

namespace cssvr {

struct LookDecision {
  Ang3 angles;
  bool applied = false; // wrote VR look over the game view
  const char* reason = "game";
};

/// cssvr_stop / STOPPING / LOSS used to keep the last HMD sample, so the next
/// session_ok applied last-session look before the first locate.
inline void Look_DropHmd(bool should_run, Pose* cache) {
  if (!should_run && cache) *cache = {};
}

inline Pose Look_TakeHmd(bool should_run, const Pose& cache) {
  if (!should_run) return {};
  return cache;
}

inline LookDecision Look_Decide(const Pose& hmd, const GunPose* gun, bool firing,
                                const Ang3& game, float yaw_off = 0.f) {
  LookDecision d;
  d.angles = game;
  if (firing && gun && gun->valid) {
    d.angles = VectorAngles(gun->forward);
    d.angles.p = AngleNormalize(d.angles.p);
    d.angles.y = AngleNormalize(d.angles.y);
    d.applied = true;
    d.reason = "snap_fire";
    return d;
  }
  if (hmd.valid) {
    d.angles = hmd.ang;
    d.angles.y = AngleNormalize(d.angles.y + yaw_off);
    d.applied = true;
    d.reason = "hmd";
    return d;
  }
  d.reason = "game";
  return d;
}

} // namespace cssvr
