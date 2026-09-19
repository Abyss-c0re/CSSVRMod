#pragma once
// HMD drives look. Snap-on-fire may override. Invalid HMD keeps the game view.
#include "aim.hpp"

namespace cssvr {

struct LookDecision {
  Ang3 angles;
  bool applied = false; // wrote VR look over the game view
  const char* reason = "game";
};

inline LookDecision Look_Decide(const Pose& hmd, const GunPose* gun, bool firing,
                                const Ang3& game, float yaw_off = 0.f) {
  LookDecision d;
  d.angles = game;
  if (firing && gun && gun->valid) {
    d.angles = VectorAngles(gun->forward);
    d.applied = true;
    d.reason = "snap_fire";
    return d;
  }
  if (hmd.valid) {
    d.angles = hmd.ang;
    d.angles.y += yaw_off; // stick locomotion; RenderView used to clobber CreateMove yaw
    d.applied = true;
    d.reason = "hmd";
    return d;
  }
  d.reason = "game";
  return d;
}

} // namespace cssvr
