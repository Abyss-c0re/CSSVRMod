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

/// Two copies of one framebuffer are not dual. Stay MONO (no pose IPD).
inline bool DualPaint_AcceptPair(bool painted_dual, bool worlds_differ) {
  return painted_dual && worlds_differ;
}

// Hook live, both paints ran, but eye copies missed. One-shot after a short hold
// so the first loading frames do not toast. Never abort VR (stay MONO).
struct DualCaptureToastIn {
  int paints = 0;
  int captures = 0;
  bool painted_dual = false;
  int incomplete_frames = 0;
  bool already_shown = false;
};

struct DualCaptureToast {
  bool should_toast = false;
  bool abort_vr = false;
  const char* reason = "idle";
  const char* copy = "";
  const char* label = "RV · IDLE";
};

inline int DualCapture_MissHold() { return 8; }

inline const char* DualCapture_MissCopy() {
  return "Dual paint captured fewer than 2 eyes — staying MONO. Check the RT copy.";
}

/// One dual attempt per present. VK must clear this on vkQueuePresentKHR.
struct DualPaintFrameGate {
  bool did_frame = false;
};

inline bool DualPaint_BeginFrame(DualPaintFrameGate* g) {
  if (!g || g->did_frame) return false;
  g->did_frame = true;
  return true;
}

inline void DualPaint_OnPresent(DualPaintFrameGate* g) {
  if (g) g->did_frame = false;
}

inline DualCaptureToast DualCapture_ToastDecide(const DualCaptureToastIn& in) {
  DualCaptureToast t;
  t.abort_vr = false;
  if (in.painted_dual) {
    t.reason = "dual_ipd_origin";
    t.label = "RV · 2CAP";
    return t;
  }
  const bool incomplete = (in.paints == 2 && in.captures < 2);
  t.reason = incomplete ? "incomplete_dual" : "idle";
  t.copy = DualCapture_MissCopy();
  t.label = incomplete ? "RV · NO CAP" : "RV · IDLE";
  t.should_toast = incomplete && !in.already_shown && in.incomplete_frames >= DualCapture_MissHold();
  return t;
}

} // namespace cssvr
