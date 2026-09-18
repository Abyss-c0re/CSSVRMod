#include "cssvrmod/source_if.hpp"
#include "cssvrmod/engine_trace.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/module_base.hpp"
#include "cssvrmod/toast.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

namespace {
cssvr::EngineCmdFilter g_cmd_filter = nullptr;
using ClientCmdFn = void (*)(void*, const char*);
ClientCmdFn g_real_cmd = nullptr;
ClientCmdFn g_orig_unrestricted = nullptr;
bool g_cmd_wrapped = false;

void FilterOrOrig(void* eng, const char* cmd, ClientCmdFn orig) {
  if (g_cmd_filter && cmd && g_cmd_filter(cmd)) return;
  if (orig) orig(eng, cmd);
}

void WrappedClientCmd(void* eng, const char* cmd) { FilterOrOrig(eng, cmd, g_real_cmd); }

void WrappedClientCmdUnrestricted(void* eng, const char* cmd) {
  FilterOrOrig(eng, cmd, g_orig_unrestricted);
}

bool ProtectSlot(void* p, bool wr) {
  long page = sysconf(_SC_PAGESIZE);
  if (page < 4096) page = 4096;
  uintptr_t pg = (uintptr_t)p & ~((uintptr_t)page - 1);
  int prot = wr ? (PROT_READ | PROT_WRITE) : (PROT_READ | PROT_EXEC);
  return mprotect((void*)pg, (size_t)page, prot) == 0;
}

void PatchCmdSlot(ClientCmdFn* slot, ClientCmdFn wrap, ClientCmdFn* orig) {
  if (!slot || !*slot || !wrap || !orig || *orig) return;
  *orig = *slot;
  ProtectSlot(slot, true);
  *slot = wrap;
  ProtectSlot(slot, false);
}

void InstallCmdWrap(void* engine) {
  if (g_cmd_wrapped || !engine) return;
  auto** vt = *reinterpret_cast<ClientCmdFn**>(engine);
  if (!vt) return;
  PatchCmdSlot(&vt[cssvr::kEngineClientCmdSlot], &WrappedClientCmd, &g_real_cmd);
  PatchCmdSlot(&vt[cssvr::kEngineClientCmdUnrestrictedSlot], &WrappedClientCmdUnrestricted,
               &g_orig_unrestricted);
  if (!g_real_cmd && !g_orig_unrestricted) return;
  g_cmd_wrapped = true;
  if (FILE* f = std::fopen("/tmp/cssvrmod.log", "a")) {
    std::fprintf(f, "cssvr cmd wrap slot7=%d slot106=%d\n", g_real_cmd ? 1 : 0,
                 g_orig_unrestricted ? 1 : 0);
    std::fclose(f);
  }
}
} // namespace

namespace cssvr {

static const char* kEngineNames[] = {"VEngineClient014", "VEngineClient013", nullptr};
static const char* kClientNames[] = {"VClient017", "VClient016", "VClient015", nullptr};
static const char* kTraceNames[] = {"EngineTraceClient003", "EngineTraceClient004", nullptr};
static const char* kCvarNames[] = {"VEngineCvar007", "VEngineCvar004", nullptr};

void* ProbeNamed(CreateInterfaceFn fn, const char* const* names, const char** used) {
  if (!fn || !names) return nullptr;
  for (int i = 0; names[i]; ++i) {
    int rc = 1;
    void* p = fn(names[i], &rc);
    if (p) {
      if (used) *used = names[i];
      return p;
    }
  }
  return nullptr;
}

class WorldOnlyFilter {
public:
  virtual bool ShouldHitEntity(void*, int) { return false; }
  virtual int GetTraceType() const { return kTraceWorldOnly; }
};

using TraceRayFn = void (*)(void*, const RayBlob*, unsigned, void*, void*);

static bool TraceRaySelfTest(void* trace, int idx) {
  if (!trace || idx < 0) return false;
  auto** vt = *reinterpret_cast<TraceRayFn**>(trace);
  if (!vt || !vt[idx]) return false;
  Dl_info info{};
  if (!dladdr(reinterpret_cast<void*>(vt[idx]), &info) || !info.dli_fname ||
      !std::strstr(info.dli_fname, "engine.so"))
    return false;
  RayBlob ray{};
  Ray_InitHull(&ray, {0.f, 0.f, 128.f}, {0.f, 0.f, -256.f}, {0, 0, 0}, {0, 0, 0});
  unsigned char tr[128]{};
  WorldOnlyFilter filter;
  vt[idx](trace, &ray, kMaskSolid, &filter, tr);
  float frac = 2.f;
  std::memcpy(&frac, tr + kTrFraction, 4);
  Vec3 end{};
  std::memcpy(&end, tr + kTrEnd, 12);
  return TraceBlobSane(frac, end);
}

static bool ScreenSelfTest(void* engine, int* w, int* h) {
  if (!engine) return false;
  // IVEngineClient::GetScreenSize is index 5 on 2013 / CSS 64-bit.
  using GetScreenSizeFn = void (*)(void*, int&, int&);
  auto** vt = *reinterpret_cast<GetScreenSizeFn**>(engine);
  if (!vt || !vt[5]) return false;
  int ww = -1, hh = -1;
  vt[5](engine, ww, hh);
  if (w) *w = ww;
  if (h) *h = hh;
  return ww > 0 && ww < 16384 && hh > 0 && hh < 16384;
}

bool ProbeEngineFromFactories(CreateInterfaceFn engineFn, CreateInterfaceFn clientFn,
                              EngineIf& out) {
  out = EngineIf{};
  if (!engineFn) {
    out.reason = "no_engine_factory";
    return false;
  }
  out.engine = ProbeNamed(engineFn, kEngineNames, &out.engine_ver);
  out.trace = ProbeNamed(engineFn, kTraceNames, &out.trace_ver);
  out.cvar = ProbeNamed(engineFn, kCvarNames, nullptr);
  if (clientFn) out.client = ProbeNamed(clientFn, kClientNames, &out.client_ver);
  if (!out.engine) {
    out.reason = "no_vengineclient";
    return false;
  }
  out.ok = true;
  out.reason = "probed";
  return true;
}

bool ProbeLiveEngine(EngineIf& out) {
  out = EngineIf{};
  CreateInterfaceFn eng = nullptr;
  CreateInterfaceFn cli = nullptr;
  // Path-loaded engine.so / client.so. Never dlopen(nullptr) — that is the launcher.
  const CssInstall inst = FindCssInstall();
  void* e = Module_SoHandle("engine.so", inst.found ? inst.engine_so.c_str() : nullptr);
  void* c = Module_SoHandle("client.so", inst.found ? inst.client_so.c_str() : nullptr);
  if (e) eng = reinterpret_cast<CreateInterfaceFn>(dlsym(e, "CreateInterface"));
  if (c) cli = reinterpret_cast<CreateInterfaceFn>(dlsym(c, "CreateInterface"));
  if (!eng) {
    out.reason = "no_createinterface";
    return false;
  }
  if (!ProbeEngineFromFactories(eng, cli, out)) return false;
  out.screen_ok = ScreenSelfTest(out.engine, &out.screen_w, &out.screen_h);
  static bool ang_toast = false;
  auto note_angles_toast = [&](bool have_engine, bool selftest_ok) {
    EngineAnglesToastIn in;
    in.have_engine = have_engine;
    in.selftest_ok = selftest_ok;
    in.probed = true;
    in.already_shown = ang_toast;
    const EngineAnglesToast t = EngineAngles_ToastDecide(in);
    if (!t.should_toast) return;
    ang_toast = true;
    if (FILE* f = std::fopen("/tmp/cssvrmod.log", "a")) {
      std::fprintf(f, "cssvr toast %s %s\n", t.label, t.copy);
      std::fclose(f);
    }
    Toast_FireDesktop(t.copy);
  };
  if (out.engine) {
    auto** vt = *reinterpret_cast<ViewAngFn**>(out.engine);
    // 2013/CSS64 layout that already matches GetScreenSize=5 / ClientCmd=7.
    constexpr int kGet = 19, kSet = 20;
    static int angles_probe = 0; // 0 idle  1 failed  2 ok
    static int ok_get = -1, ok_set = -1;
    if (angles_probe == 2) {
      out.angles_ok = true;
      out.get_angles_idx = ok_get;
      out.set_angles_idx = ok_set;
    } else if (angles_probe == 0) {
      bool ok = false;
      if (vt && vt[kGet] && vt[kSet]) {
        Dl_info gi{}, si{};
        const bool in_eng = dladdr(reinterpret_cast<void*>(vt[kGet]), &gi) && gi.dli_fname &&
                            std::strstr(gi.dli_fname, "engine.so") &&
                            dladdr(reinterpret_cast<void*>(vt[kSet]), &si) && si.dli_fname &&
                            std::strstr(si.dli_fname, "engine.so");
        if (in_eng && ViewAnglesRoundtripOk(out.engine, vt[kGet], vt[kSet])) {
          out.angles_ok = true;
          out.get_angles_idx = kGet;
          out.set_angles_idx = kSet;
          ok_get = kGet;
          ok_set = kSet;
          angles_probe = 2;
          ok = true;
        }
      }
      if (!ok) {
        angles_probe = 1; // do not yank view again
        note_angles_toast(true, false);
      }
    }
  } else {
    note_angles_toast(false, false);
  }
  static bool tr_toast = false;
  auto note_trace_toast = [&](bool have_iface, bool selftest_ok) {
    EngineTraceToastIn in;
    in.have_iface = have_iface;
    in.selftest_ok = selftest_ok;
    in.probed = true;
    in.already_shown = tr_toast;
    const EngineTraceToast t = EngineTrace_ToastDecide(in);
    if (!t.should_toast) return;
    tr_toast = true;
    if (FILE* f = std::fopen("/tmp/cssvrmod.log", "a")) {
      std::fprintf(f, "cssvr toast %s %s\n", t.label, t.copy);
      std::fclose(f);
    }
    Toast_FireDesktop(t.copy);
  };
  if (out.trace) {
    static int trace_probe = 0; // 0 idle  1 fail  2 ok
    static int ok_idx = -1;
    if (trace_probe == 2) {
      out.trace_ok = true;
      out.trace_ray_idx = ok_idx;
    } else if (trace_probe == 0) {
      const int a = (out.trace_ver && std::strstr(out.trace_ver, "004")) ? 5 : 4;
      const int b = (a == 4) ? 5 : 4;
      int hit = -1;
      if (TraceRaySelfTest(out.trace, a)) hit = a;
      else if (TraceRaySelfTest(out.trace, b)) hit = b;
      if (hit >= 0) {
        out.trace_ok = true;
        out.trace_ray_idx = hit;
        ok_idx = hit;
        trace_probe = 2;
      } else {
        trace_probe = 1;
        note_trace_toast(true, false);
      }
    }
  } else {
    note_trace_toast(false, false);
  }
  if (out.trace_ok) out.reason = "probed_trace_ok";
  else if (out.angles_ok) out.reason = "probed_angles_ok";
  else out.reason = out.screen_ok ? "probed_screen_ok" : "probed_no_screen";
  if (out.engine) InstallCmdWrap(out.engine);
  return true;
}

void EngineSetCmdFilter(EngineCmdFilter f) { g_cmd_filter = f; }

bool EngineClientCmd(const EngineIf& e, const char* cmd) {
  if (!e.engine || !e.screen_ok || !cmd) return false;
  InstallCmdWrap(e.engine);
  if (g_cmd_filter && g_cmd_filter(cmd)) return true;
  if (g_real_cmd) {
    g_real_cmd(e.engine, cmd);
    return true;
  }
  auto** vt = *reinterpret_cast<ClientCmdFn**>(e.engine);
  if (!vt || !vt[kEngineClientCmdSlot]) return false;
  vt[kEngineClientCmdSlot](e.engine, cmd);
  return true;
}

bool EngineGetViewAngles(const EngineIf& e, Ang3* out) {
  if (!e.engine || !e.angles_ok || !out || e.get_angles_idx < 0) return false;
  auto** vt = *reinterpret_cast<ViewAngFn**>(e.engine);
  if (!vt || !vt[e.get_angles_idx]) return false;
  vt[e.get_angles_idx](e.engine, out);
  return ViewAnglesSane(*out);
}

bool EngineSetViewAngles(const EngineIf& e, const Ang3& a) {
  if (!e.engine || !e.angles_ok || e.set_angles_idx < 0) return false;
  if (!ViewAnglesSane(a)) return false;
  auto** vt = *reinterpret_cast<ViewAngFn**>(e.engine);
  if (!vt || !vt[e.set_angles_idx]) return false;
  Ang3 tmp = a;
  vt[e.set_angles_idx](e.engine, &tmp);
  return true;
}

static TraceHit LiveHull(void* trace, int idx, Vec3 start, Vec3 end, Vec3 mins, Vec3 maxs) {
  TraceHit miss;
  miss.fraction = 1.f;
  miss.end_pos = end;
  if (!trace || idx < 0) return miss;
  auto** vt = *reinterpret_cast<TraceRayFn**>(trace);
  if (!vt || !vt[idx]) return miss;
  RayBlob ray{};
  Ray_InitHull(&ray, start, end, mins, maxs);
  unsigned char tr[128]{};
  WorldOnlyFilter filter;
  vt[idx](trace, &ray, kMaskSolid, &filter, tr);
  TraceHit hit = TraceHitFromBlob(tr, sizeof(tr));
  if (!hit.hit && !hit.start_solid && !hit.all_solid) hit.end_pos = end;
  return hit;
}

TraceFn EngineMakeTraceFn(const EngineIf& e) {
  if (!e.trace_ok || !e.trace || e.trace_ray_idx < 0) return {};
  void* tr = e.trace;
  const int idx = e.trace_ray_idx;
  return [tr, idx](Vec3 s, Vec3 epos, Vec3 mins, Vec3 maxs) {
    return LiveHull(tr, idx, s, epos, mins, maxs);
  };
}

} // namespace cssvr
