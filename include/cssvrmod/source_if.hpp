#pragma once
// Source CreateInterface probe. No guessed vtable calls unless a self-test passes.
#include "collision.hpp"
#include "module_base.hpp"
#include "vec3.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
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
  const char* cvar_ver = "";
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
                              EngineIf& out, CreateInterfaceFn vstdlibFn = nullptr);

/// Live process: dlsym CreateInterface from already-loaded engine/client/vstdlib.
bool ProbeLiveEngine(EngineIf& out);

/// ClientCmd if GetScreenSize(index 5) self-test passed. Never call blindly.
bool EngineClientCmd(const EngineIf& e, const char* cmd);

/// Hook installs a console filter so `cssvr_start` never reaches the engine unknown-cmd path.
using EngineCmdFilter = bool (*)(const char* cmd);
void EngineSetCmdFilter(EngineCmdFilter f);

/// CSS64 CEngineClient: 7 = ClientCmd (restricted). 106 = ClientCmd_Unrestricted
/// (`mov rdi,rsi; jmp Cbuf_AddText`). Typed console uses 106, not 7.
constexpr int kEngineClientCmdSlot = 7;
constexpr int kEngineClientCmdUnrestrictedSlot = 106;

struct EngineCmdWrapPlan {
  int slots[4]{};
  int n = 0;
};

inline EngineCmdWrapPlan EngineCmd_WrapPlan() {
  EngineCmdWrapPlan p;
  p.slots[p.n++] = kEngineClientCmdSlot;
  p.slots[p.n++] = kEngineClientCmdUnrestrictedSlot;
  return p;
}

inline bool EngineCmd_ShouldWrapSlot(int slot) {
  return slot == kEngineClientCmdSlot || slot == kEngineClientCmdUnrestrictedSlot;
}

/// Stop retrying only when unrestricted is on, or slot 106 is present but not engine.so.
inline bool EngineCmd_WrapComplete(bool have_unrestricted, bool slot106_present,
                                   bool slot106_in_engine) {
  if (have_unrestricted) return true;
  return slot106_present && !slot106_in_engine;
}

/// Splash wrap used to skip ICvar. CCvar exists before engine registers `echo`;
/// a one-shot ProbeLiveEngine after slot 106 never lists cssvr_*.
inline bool EngineProbe_PresentDone(bool cmd_wrap_ready, bool icvar_ready) {
  return cmd_wrap_ready && icvar_ready;
}

/// ClientCmd_Unrestricted is `mov rdi,rsi; jmp rel32` to Cbuf_AddText.
/// Typed console calls Cbuf_AddText, not the IVEngineClient wrapper.
inline void* EngineCmd_DecodeCbuf(const unsigned char* p) {
  if (!p) return nullptr;
  // 48 89 f7 e9 xx xx xx xx
  if (p[0] == 0x48 && p[1] == 0x89 && p[2] == 0xf7 && p[3] == 0xe9) {
    int32_t rel = 0;
    std::memcpy(&rel, p + 4, 4);
    return (void*)(p + 8 + rel);
  }
  if (p[0] == 0xe9) {
    int32_t rel = 0;
    std::memcpy(&rel, p + 1, 4);
    return (void*)(p + 5 + rel);
  }
  return nullptr;
}

/// Inline hook overwrites this many bytes (`movabs rax; jmp rax`).
inline int EngineCmd_CbufStealBytes() { return 12; }

/// One allowlisted, position-independent insn. 0 = unknown / RIP-relative / truncated.
/// CSS Cbuf starts `push rbp; lea rcx,[rip+disp]` — that lea must not be stolen.
inline int EngineCmd_CbufInsnLen(const unsigned char* p, int left) {
  if (!p || left <= 0) return 0;
  if (p[0] == 0x90) return 1;                         // nop
  if (p[0] == 0x55 || p[0] == 0x53) return 1;         // push rbp / rbx
  if (p[0] == 0x31 && left >= 2 && p[1] == 0xc0) return 2; // xor eax,eax
  if (p[0] == 0x41 && left >= 2 && p[1] >= 0x54 && p[1] <= 0x57) return 2; // push r12-r15
  if (left >= 3 && p[0] == 0x48 && p[1] == 0x89 && p[2] == 0xe5) return 3; // mov rbp,rsp
  if (left >= 4 && p[0] == 0x48 && p[1] == 0x83 && p[2] == 0xec) return 4; // sub rsp,imm8
  if (left >= 4 && p[0] == 0xf3 && p[1] == 0x0f && p[2] == 0x1e && p[3] == 0xfa) return 4; // endbr64
  return 0;
}

/// True only if [p, p+n) is an exact run of relocatable insns. Else the trampoline
/// splits an insn or relocates a RIP-relative lea — every Cbuf_AddText would crash.
inline bool EngineCmd_CbufStealOk(const unsigned char* p, int n) {
  if (!p || n != EngineCmd_CbufStealBytes()) return false;
  int i = 0;
  while (i < n) {
    const int len = EngineCmd_CbufInsnLen(p + i, n - i);
    if (len <= 0) return false;
    i += len;
  }
  return i == n;
}

bool EngineCmd_WrapReady();
bool ICvar_CmdsReady();

/// VEngineCvar004: IAppSystem(5) + Allocate + Register = 6. FindCommand = 14.
constexpr int kCvarRegister004 = 6;
constexpr int kCvarFindCommand004 = 14;

inline bool ICvar_Layout004(const char* ver) {
  return ver && std::strstr(ver, "004");
}

/// CSS ICvar is CCvar in libvstdlib.so. engine.so only exposes VCvarQuery001.
/// Same basename rule as maps (`(deleted)` stripped).
inline bool ICvar_FnInModule(const char* fname) {
  if (!fname || !fname[0]) return false;
  const char* end = fname + std::strlen(fname);
  return Maps_LineHasNeedle(fname, end, "libvstdlib.so") || Maps_LineHasNeedle(fname, end, "engine.so");
}

/// Sibling of engine.so in bin/linux64. Empty if engine path has no slash.
inline bool ICvar_VstdlibBesideEngine(const char* engine_so, char* out, int n) {
  if (!out || n < 16 || !engine_so || !engine_so[0]) return false;
  const char* slash = std::strrchr(engine_so, '/');
  if (!slash) return false;
  const int dir_n = (int)(slash - engine_so + 1);
  if (dir_n + 14 > n) return false;
  std::memcpy(out, engine_so, (size_t)dir_n);
  std::snprintf(out + dir_n, (size_t)(n - dir_n), "libvstdlib.so");
  return true;
}

/// Rebuild `name args` from a Source CCommand (argc + argv0size + ArgS[512]).
/// Insane blobs fall back to the verb name so cssvr_start still fires.
inline bool ICvar_DispatchLine(const char* name, const void* cmd, char* out, int n) {
  if (!out || n < 8 || !name || !name[0]) return false;
  out[0] = 0;
  int argc = 0, argv0 = -1;
  if (cmd) {
    std::memcpy(&argc, cmd, 4);
    std::memcpy(&argv0, static_cast<const char*>(cmd) + 4, 4);
  }
  const char* args = "";
  if (cmd && argc >= 1 && argc <= 16 && argv0 >= 0 && argv0 < 128) {
    args = static_cast<const char*>(cmd) + 8 + argv0;
  }
  if (args && args[0] && (unsigned char)args[0] >= 32 && (unsigned char)args[0] < 127)
    std::snprintf(out, (size_t)n, "%s %s", name, args);
  else
    std::snprintf(out, (size_t)n, "%s", name);
  return out[0] != 0;
}

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
