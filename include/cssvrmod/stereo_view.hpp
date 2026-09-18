#pragma once
// gmod dual RenderView law: same angles, origin ± head Right × halfIPD.
// IPD lives in those two rasters. VIEW pose IPD on top is a second plane.
#include "calib.hpp"
#include "vec3.hpp"

namespace cssvr {

constexpr float kInchesPerMeter = 39.3700787f;

struct StereoViewIn {
  Vec3 origin; // cyclopean, Source inches
  Ang3 angles; // shared HMD / game view — never per-eye rotation
  float fov = 90.f;
  float aspect = 16.f / 9.f;
  Calib calib;
  float world_scale = kInchesPerMeter;
};

struct EyeView {
  Vec3 origin;
  Ang3 angles;
  float fov = 90.f;
  float aspect = 16.f / 9.f;
  int eye = 0;          // 0 left, 1 right
  float half_ipd = 0.f; // Source inches
  float pose_x = 0.f;   // VIEW-space metres; always 0 on the lens blit
};

struct StereoViewPlan {
  bool valid = false;
  bool rigid = true;
  bool painted_dual = false;
  float half_ipd = 0.f;   // Source inches
  float half_ipd_m = 0.f; // metres (submit)
  EyeView left;
  EyeView right;
  const char* reason = "idle";
};

/// World half-IPD in Source inches. Dual paint uses full ipd_m.
/// eyescale is UV-only after cycle 1 — a Vision 0.13 must not crush camera sep.
inline float StereoView_HalfIpdInches(const Calib& raw, float world_scale = kInchesPerMeter,
                                      bool painted_dual = false) {
  const Calib c = ClampCalib(raw);
  float scale = world_scale;
  if (scale < 0.01f) scale = kInchesPerMeter;
  const float eye = painted_dual ? 1.f : c.eyescale;
  return c.ipd_m * scale * 0.5f * eye;
}

/// VIEW-space pose X. Always 0: the blit sits on the lenses (cycle 1).
/// Dual IPD is the two world origins. Pose IPD on those rasters is two planes.
inline float StereoView_SubmitPoseX(const Calib& raw, int eye, bool painted_dual) {
  (void)raw;
  (void)eye;
  (void)painted_dual;
  return 0.f;
}

/// Two IPD-offset cameras (gmod method). painted_dual must mean two world paints.
inline StereoViewPlan StereoView_Decide(const StereoViewIn& in, bool painted_dual) {
  StereoViewPlan p;
  p.valid = true;
  p.rigid = true;
  p.painted_dual = painted_dual;
  const Calib c = ClampCalib(in.calib);
  p.half_ipd = StereoView_HalfIpdInches(c, in.world_scale, painted_dual);
  p.half_ipd_m = painted_dual ? (c.ipd_m * 0.5f) : (c.ipd_m * 0.5f * c.eyescale);

  Vec3 right;
  AngleVectors(in.angles, nullptr, &right, nullptr);

  auto fill = [&](EyeView& ev, int eye, float sign) {
    ev.eye = eye;
    ev.angles = in.angles;
    ev.fov = in.fov;
    ev.aspect = in.aspect;
    ev.half_ipd = p.half_ipd;
    ev.origin = painted_dual ? (in.origin + right * (sign * p.half_ipd)) : in.origin;
    ev.pose_x = StereoView_SubmitPoseX(c, eye, painted_dual);
  };
  // SWAP is a camera swap. Pose/UV IPD stay 0 (no second plane).
  const float sign_l = c.swap_eyes ? +1.f : -1.f;
  const float sign_r = c.swap_eyes ? -1.f : +1.f;
  fill(p.left, 0, sign_l);
  fill(p.right, 1, sign_r);
  p.reason = painted_dual ? "dual_ipd_origin" : "mono_identity_pose";
  return p;
}

} // namespace cssvr
