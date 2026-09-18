#pragma once
// Drive two IPD-offset world paints. painted_dual only if both paints + both captures.
#include "stereo_view.hpp"
#include <functional>

namespace cssvr {

using DualPaintFn = std::function<bool(const EyeView& eye)>;
using DualCaptureFn = std::function<bool(int eye)>;

struct DualPaintResult {
  int paints = 0;
  int captures = 0;
  bool painted_dual = false;
  Vec3 left_origin;
  Vec3 right_origin;
  const char* reason = "idle";
};

/// paint(eye) must draw that camera. capture(eye) must copy that framebuffer.
/// One paint or a failed capture leaves painted_dual false (heresy gate).
inline DualPaintResult DualPaint_Run(const StereoViewIn& in, const DualPaintFn& paint,
                                     const DualCaptureFn& capture) {
  DualPaintResult r;
  if (!paint) {
    r.reason = "no_paint";
    return r;
  }
  const auto plan = StereoView_Decide(in, true);
  r.left_origin = plan.left.origin;
  r.right_origin = plan.right.origin;
  if (paint(plan.left)) r.paints++;
  if (capture && capture(0)) r.captures++;
  if (paint(plan.right)) r.paints++;
  if (capture && capture(1)) r.captures++;
  r.painted_dual = (r.paints == 2 && r.captures == 2);
  r.reason = r.painted_dual ? "dual_ipd_origin" : "incomplete_dual";
  return r;
}

} // namespace cssvr
