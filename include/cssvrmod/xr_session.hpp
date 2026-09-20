#pragma once
// Session reason honesty. session_ok only after BeginSession, never on warmup or stop.
// Matches XrSessionState integers from openxr.h without pulling the SDK into tests.
#include <cstring>

namespace cssvr {

constexpr int kXrStateUnknown = 0;
constexpr int kXrStateIdle = 1;
constexpr int kXrStateReady = 2;
constexpr int kXrStateSynchronized = 3;
constexpr int kXrStateVisible = 4;
constexpr int kXrStateFocused = 5;
constexpr int kXrStateStopping = 6;
constexpr int kXrStateLossPending = 7;
constexpr int kXrStateExiting = 8;

enum class XrSessionPhase {
  none = 0,
  created,  // xrCreateSession succeeded; not begun
  ready,    // READY event; BeginSession not yet ok
  running,  // BeginSession succeeded — only then session_ok
  stopping, // STOPPING / EndSession
  lost,     // LOSS_PENDING / EXITING / instance loss
};

inline XrSessionPhase XrSession_FromState(int xr_state, bool session_handle, bool running) {
  if (!session_handle) return XrSessionPhase::none;
  if (xr_state == kXrStateLossPending || xr_state == kXrStateExiting) return XrSessionPhase::lost;
  if (xr_state == kXrStateStopping) return XrSessionPhase::stopping;
  if (running) return XrSessionPhase::running;
  if (xr_state == kXrStateReady) return XrSessionPhase::ready;
  return XrSessionPhase::created;
}

inline const char* XrSession_Reason(XrSessionPhase p) {
  switch (p) {
    case XrSessionPhase::created:
      return "session_created";
    case XrSessionPhase::ready:
      return "session_ready";
    case XrSessionPhase::running:
      return "session_ok";
    case XrSessionPhase::stopping:
      return "session_stopping";
    case XrSessionPhase::lost:
      return "no_hmd";
    default:
      return "no_session";
  }
}

inline bool XrSession_IsOk(XrSessionPhase p) { return p == XrSessionPhase::running; }

inline bool XrSession_IsOkReason(const char* reason) {
  return reason && std::strcmp(reason, "session_ok") == 0;
}

/// Desktop MONO stamp while a session exists but dual paint has not.
inline bool XrSession_ShowsMono(const char* reason) {
  if (!reason || !reason[0]) return false;
  return std::strcmp(reason, "session_ok") == 0 || std::strcmp(reason, "session_created") == 0 ||
         std::strcmp(reason, "session_ready") == 0;
}

/// Keep stopping/lost; otherwise the begin miss is warmup or a stopped session.
inline const char* XrSession_BeginMiss(bool session, bool running, const char* current) {
  if (!session) return (current && current[0]) ? current : "no_session";
  if (running) return (current && current[0]) ? current : "session_ok";
  if (current && (std::strcmp(current, "session_stopping") == 0 ||
                  std::strcmp(current, "no_hmd") == 0 || std::strcmp(current, "shutdown") == 0))
    return current;
  return "session_not_running";
}

/// shouldRender=false after Wait+Begin is a skip, not a failed healthy session.
enum class XrBeginKind { miss, skip, ready };

inline XrBeginKind XrSession_BeginKind(bool running, bool wait_ok, bool should_render) {
  if (!running || !wait_ok) return XrBeginKind::miss;
  if (!should_render) return XrBeginKind::skip;
  return XrBeginKind::ready;
}

/// Count a submit fail only while we claim session_ok and we actually tried.
inline bool XrSubmit_CountFail(const char* reason, bool submit_ok, bool skipped) {
  if (submit_ok || skipped) return false;
  return XrSession_IsOkReason(reason);
}

/// Shutdown must not re-arm the no-HMD toast (one-shot per process).
inline bool XrSession_ResetToastOnShutdown() { return false; }

/// Leftover mailbox must not submit when !session_ok (cycle 235).
/// Events must still be pumped while XR is wanted, or READY after STOPPING is never seen.
inline bool XrWorker_ShouldSubmit(bool xr_wanted, bool session_ok) {
  return xr_wanted && session_ok;
}
inline bool XrWorker_ShouldPump(bool xr_wanted) { return xr_wanted; }

/// Cycle 235 skipped submit while !ok, but a mailbox sitting through STOPPING→READY
/// submitted last-session rasters. Drop have/dual so session_ok cannot resurrect them.
inline bool XrMailbox_Keep(bool xr_wanted, bool session_ok) {
  return XrWorker_ShouldSubmit(xr_wanted, session_ok);
}
inline void XrMailbox_Drop(bool keep, bool* have, bool* dual) {
  if (keep) return;
  if (have) *have = false;
  if (dual) *dual = false;
}

/// Epoch bumps when leaving session_ok. READY must not accept a push from the old epoch.
inline int XrSession_BumpEpoch(bool was_ok, bool now_ok, int epoch) {
  if (was_ok && !now_ok) return epoch + 1;
  return epoch;
}
inline bool XrMailbox_Accept(bool keep, int push_epoch, int live_epoch) {
  return keep && push_epoch == live_epoch;
}

} // namespace cssvr
