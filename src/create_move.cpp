#include "cssvrmod/create_move.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/usercmd.hpp"

#include <cstdarg>
#include <cstdio>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cssvr {
namespace {

using CreateMoveFn = bool (*)(void*, float, void*);
CreateMoveFn g_orig = nullptr;
CreateMoveLoc g_loc;
bool g_installed = false;

void Logf(const char* fmt, ...) {
  FILE* f = std::fopen("/tmp/cssvrmod.log", "a");
  if (!f) return;
  va_list ap;
  va_start(ap, fmt);
  std::vfprintf(f, fmt, ap);
  va_end(ap);
  std::fputc('\n', f);
  std::fclose(f);
}

bool ProtectWrite(void* p, bool wr) {
  const long page = sysconf(_SC_PAGESIZE);
  if (page <= 0) return false;
  auto addr = reinterpret_cast<uintptr_t>(p) & ~(uintptr_t)(page - 1);
  const int prot = PROT_READ | (wr ? PROT_WRITE : 0);
  return mprotect(reinterpret_cast<void*>(addr), (size_t)page, prot) == 0;
}

bool HookedCreateMove(void* self, float dt, void* cmd) {
  UserCmdOverlay ov;
  if (cmd && UserCmd_PeekOverlay(&ov)) {
    UserCmdFields f;
    if (UserCmd_Detect(cmd, 64, &f)) UserCmd_Apply(cmd, 64, f, ov);
  }
  if (!g_orig) return true;
  return g_orig(self, dt, cmd);
}

uintptr_t ClientBase() {
  void* h = dlopen("client.so", RTLD_NOW | RTLD_NOLOAD);
  if (!h) return 0;
  void* ci = dlsym(h, "CreateInterface");
  if (!ci) return 0;
  Dl_info info{};
  if (!dladdr(ci, &info) || !info.dli_fbase) return 0;
  return reinterpret_cast<uintptr_t>(info.dli_fbase);
}

} // namespace

bool UserCmd_HookLive() {
  if (g_installed) return true;
  CssInstall inst = FindCssInstall();
  if (!inst.found) {
    Logf("createmove skip: no css");
    return false;
  }
  if (!LocateCreateMoveFile(inst.client_so.c_str(), &g_loc) || !g_loc.found) {
    Logf("createmove locate fail %s", g_loc.reason);
    return false;
  }
  const uintptr_t base = ClientBase();
  if (!base) {
    Logf("createmove no client base");
    return false;
  }
  g_orig = reinterpret_cast<CreateMoveFn>(base + g_loc.fn_rva);
  int patched = 0;
  for (uint64_t slot : g_loc.slot_rva) {
    auto* p = reinterpret_cast<void**>(base + slot);
    if (*p != reinterpret_cast<void*>(g_orig)) continue;
    if (!ProtectWrite(p, true)) continue;
    *p = reinterpret_cast<void*>(&HookedCreateMove);
    ProtectWrite(p, false);
    patched++;
  }
  g_installed = patched > 0;
  Logf("createmove hook fn=0x%llx slot=%d patched=%d", (unsigned long long)g_loc.fn_rva, g_loc.slot,
       patched);
  if (!patched) g_orig = nullptr;
  return g_installed;
}

} // namespace cssvr
