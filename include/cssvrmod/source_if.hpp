#pragma once
// Source CreateInterface probe. No guessed vtable calls unless a self-test passes.
#include "vec3.hpp"
#include <cmath>
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

/// Get/Set viewangles only after angles_ok self-test.
bool EngineGetViewAngles(const EngineIf& e, Ang3* out);
bool EngineSetViewAngles(const EngineIf& e, const Ang3& a);

} // namespace cssvr
