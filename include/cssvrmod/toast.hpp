#pragma once
// Honest XR toast (Cube G32/G39). Logs-only on no-HMD is silent death.
#include <cstring>

namespace cssvr {

struct ToastDecision {
  bool should_toast = false;
  bool abort_vr = false; // never abort start solely because toast fired
  bool path_ok = true;
  const char* reason = "idle";
  const char* risk = "none"; // none | no_hmd | no_runtime | silent_fail
  const char* copy = "";
  const char* label = "XR · IDLE";
};

inline bool Toast_RequireOnNoHmd() { return true; }
inline bool Toast_AbortVrOnFail() { return false; }
inline int Toast_Seconds() { return 10; }

inline bool Toast_IsNoHmdReason(const char* reason) {
  return reason && std::strcmp(reason, "no_hmd") == 0;
}

inline bool Toast_IsFailReason(const char* reason) {
  if (!reason || !reason[0]) return false;
  if (Toast_IsNoHmdReason(reason)) return true;
  return std::strcmp(reason, "no_loader") == 0 || std::strcmp(reason, "no_getproc") == 0 ||
         std::strcmp(reason, "create_instance") == 0 || std::strcmp(reason, "create_session") == 0 ||
         std::strcmp(reason, "no_glx") == 0 || std::strcmp(reason, "no_space") == 0 ||
         std::strcmp(reason, "swapchain") == 0 || std::strcmp(reason, "no_x_display") == 0 ||
         std::strcmp(reason, "no_fbconfig") == 0 || std::strcmp(reason, "no_visual") == 0 ||
         std::strcmp(reason, "glx_make_current") == 0;
}

inline bool Toast_ShouldShow(const char* reason, bool already_shown) {
  if (already_shown) return false;
  return Toast_IsFailReason(reason);
}

inline const char* Toast_Copy(const char* reason) {
  if (Toast_IsNoHmdReason(reason))
    return "No HMD — start SteamVR/OpenXR; put the headset on; check cable.";
  if (reason && (std::strcmp(reason, "no_loader") == 0 || std::strcmp(reason, "no_getproc") == 0))
    return "OpenXR loader missing — install libopenxr; start the runtime.";
  if (reason && std::strcmp(reason, "create_instance") == 0)
    return "OpenXR runtime not running — start SteamVR/Monado/WiVRn.";
  if (reason && std::strcmp(reason, "create_session") == 0)
    return "OpenXR session failed — close other VR apps; restart the runtime.";
  if (reason && (std::strcmp(reason, "no_glx") == 0 || std::strcmp(reason, "no_x_display") == 0 ||
                 std::strcmp(reason, "no_fbconfig") == 0 || std::strcmp(reason, "no_visual") == 0 ||
                 std::strcmp(reason, "glx_make_current") == 0))
    return "OpenXR GL bind failed — CSS window GLX missing.";
  if (reason && (std::strcmp(reason, "no_space") == 0 || std::strcmp(reason, "swapchain") == 0))
    return "OpenXR swapchain/space failed — restart the runtime.";
  if (reason && std::strcmp(reason, "session_ok") == 0) return "OpenXR session ok.";
  return "CSSVRMod XR failed — runtime / HMD?";
}

inline const char* Toast_Label(const char* reason) {
  if (Toast_IsNoHmdReason(reason)) return "XR · NO HMD";
  if (Toast_IsFailReason(reason)) return "XR · NO RUNTIME";
  if (reason && std::strcmp(reason, "session_ok") == 0) return "XR · OK";
  return "XR · IDLE";
}

inline ToastDecision Toast_Decide(const char* xr_reason, bool already_shown) {
  ToastDecision d;
  d.reason = (xr_reason && xr_reason[0]) ? xr_reason : "idle";
  d.should_toast = Toast_ShouldShow(d.reason, already_shown);
  d.abort_vr = Toast_AbortVrOnFail();
  d.copy = Toast_Copy(d.reason);
  d.label = Toast_Label(d.reason);
  if (Toast_IsNoHmdReason(d.reason)) {
    d.risk = "no_hmd";
    d.path_ok = false;
  } else if (Toast_IsFailReason(d.reason)) {
    d.risk = "no_runtime";
    d.path_ok = false;
  } else {
    d.risk = "none";
    d.path_ok = (std::strcmp(d.reason, "session_ok") == 0);
  }
  return d;
}

// toast_fired=false when a required toast was not shown (Cube silent-fail).
inline ToastDecision Toast_Audit(const char* xr_reason, bool already_shown, bool toast_fired) {
  ToastDecision d = Toast_Decide(xr_reason, already_shown);
  if (d.should_toast && !toast_fired) {
    d.risk = "silent_fail";
    d.path_ok = false;
    d.label = "XR · SILENT FAIL";
  }
  return d;
}

// Desktop notify + /tmp/cssvrmod.toast. CSSVR_TOAST=0 skips notify-send.
bool Toast_FireDesktop(const char* copy);

} // namespace cssvr
