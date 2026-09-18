#pragma once
// Source CreateInterface probe. No guessed vtable calls unless a self-test passes.
#include "collision.hpp"
#include "vec3.hpp"
#include <cmath>
#include <cstring>
#include <string>

namespace cssvr {

using CreateInterfaceFn = void* (*)(const char* name, int* returnCode);

struct EngineIf {
  void* engine = nullptr; // IVEngineClient
  void* client = nullptr; // IBaseClientDLL
  void* trace = nullptr;  // IEngineTrace
  void* cvar = nullptr;
  const char* engine_ver = "";
  const char* client_ver = "";
  const char* trace_ver = "";
  bool screen_ok = false; // GetScreenSize self-test
  int screen_w = 0, screen_h = 0;
  bool angles_ok = false; // Get/SetViewAngles roundtrip
  int get_angles_idx = -1;
  int set_angles_idx = -1;
  bool trace_ok = false; // IEngineTrace TraceRay self-test
  int trace_ray_idx = -1;
  bool ok = false;
  const char* reason = "idle";
};

using ViewAngFn = void (*)(void* engine, Ang3* ang);

inline bool ViewAnglesSane(const Ang3& a) {
  if (!std::isfinite(a.p) || !std::isfinite(a.y) || !std::isfinite(a.r)) return false;
  if (a.p < -180.f || a.p > 180.f) return false;
  if (a.y < -720.f || a.y > 720.f) return false;
  if (a.r < -180.f || a.r > 180.f) return false;
  return true;
}

/// Write probe, read back, restore. Never call live slots without this.
inline bool ViewAnglesRoundtripOk(void* obj, ViewAngFn get, ViewAngFn set) {
  if (!obj || !get || !set) return false;
  Ang3 orig{};
  get(obj, &orig);
  if (!ViewAnglesSane(orig)) return false;
  Ang3 probe{1.25f, 33.5f, 0.f};
  set(obj, &probe);
  Ang3 back{};
  get(obj, &back);
  set(obj, &orig);
  return std::fabs(back.p - probe.p) < 0.05f && std::fabs(back.y - probe.y) < 0.05f;
}

/// Try well-known CSS 64-bit interface names against a factory (testable).
void* ProbeNamed(CreateInterfaceFn fn, const char* const* names, const char** used);

bool ProbeEngineFromFactories(CreateInterfaceFn engineFn, CreateInterfaceFn clientFn,
                              EngineIf& out);

/// Live process: dlsym CreateInterface from already-loaded engine/client.
bool ProbeLiveEngine(EngineIf& out);

/// ClientCmd if GetScreenSize(index 5) self-test passed. Never call blindly.
bool EngineClientCmd(const EngineIf& e, const char* cmd);

/// Hook installs a console filter so `cssvr_start` never reaches the engine unknown-cmd path.
using EngineCmdFilter = bool (*)(const char* cmd);
void EngineSetCmdFilter(EngineCmdFilter f);

/// Get/Set viewangles only after angles_ok self-test.
bool EngineGetViewAngles(const EngineIf& e, Ang3* out);
bool EngineSetViewAngles(const EngineIf& e, const Ang3& a);

/// Hull/point sweep. Empty function if !trace_ok.
TraceFn EngineMakeTraceFn(const EngineIf& e);

// Honest toast if Get/SetViewAngles self-test fails. HMD look will not write engine yaw.
struct EngineAnglesToastIn {
  bool have_engine = false;
  bool selftest_ok = false;
  bool probed = false;
  bool already_shown = false;
};

struct EngineAnglesToast {
  bool should_toast = false;
  bool abort_vr = false;
  const char* reason = "idle";
  const char* copy = "";
  const char* label = "ANG · IDLE";
};

inline const char* EngineAngles_MissCopy(const char* reason) {
  if (reason && std::strcmp(reason, "no_engine") == 0)
    return "IVEngineClient missing — HMD look will not write engine yaw.";
  if (reason && std::strcmp(reason, "selftest_fail") == 0)
    return "ViewAngles self-test failed — HMD look will not write engine yaw.";
  return "ViewAngles unavailable — HMD look will not write engine yaw.";
}

inline EngineAnglesToast EngineAngles_ToastDecide(const EngineAnglesToastIn& in) {
  EngineAnglesToast t;
  t.abort_vr = false;
  if (!in.probed) return t;
  if (in.selftest_ok) {
    t.reason = "angles_ok";
    t.label = "ANG · OK";
    return t;
  }
  t.reason = in.have_engine ? "selftest_fail" : "no_engine";
  t.copy = EngineAngles_MissCopy(t.reason);
  t.label = "ANG · MISS";
  t.should_toast = !in.already_shown;
  return t;
}

} // namespace cssvr
