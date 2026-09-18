#include "cssvrmod/create_move.hpp"
#include "cssvrmod/hook_api.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/module_base.hpp"
#include "cssvrmod/toast.hpp"
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
bool g_attempted = false;
bool g_cm_toast = false;

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
  // Original fills the cmd from mouse/keys. Overlay after, or it is wiped.
  const bool rc = g_orig ? g_orig(self, dt, cmd) : true;
  UserCmdOverlay ov;
  if (cmd && UserCmd_PeekOverlay(&ov)) {
    UserCmdFields f;
    if (UserCmd_Detect(cmd, 64, &f)) UserCmd_Apply(cmd, 64, f, ov);
  }
  return rc;
}

uintptr_t ClientBase(const char* full_path) { return Module_ClientBase(full_path); }

void NoteCreateMoveToast(const char* reason, bool hooked) {
  CreateMoveToastIn in;
  in.locate_reason = reason;
  in.hooked = hooked;
  in.already_shown = g_cm_toast;
  const CreateMoveToast t = CreateMove_ToastDecide(in);
  if (!t.should_toast) return;
  g_cm_toast = true;
  Logf("cssvr toast %s %s", t.label, t.copy);
  Toast_FireDesktop(t.copy);
  Chrome_NoteStatus("NO CMD");
}

} // namespace

bool UserCmd_HookLive() {
  if (g_installed) return true;
  if (g_attempted) return false;
  CssInstall inst = FindCssInstall();
  if (!inst.found) {
    Logf("createmove skip: no css");
    NoteCreateMoveToast("no_css", false);
    g_attempted = true;
    return false;
  }
  if (!g_loc.found) {
    if (!LocateCreateMoveFile(inst.client_so.c_str(), &g_loc) || !g_loc.found) {
      Logf("createmove locate fail %s", g_loc.reason);
      NoteCreateMoveToast(g_loc.reason ? g_loc.reason : "no_rtti", false);
      g_attempted = true;
      return false;
    }
  }
  const uintptr_t base = ClientBase(inst.client_so.c_str());
  if (!base) {
    static int n = 0;
    if (n++ < 3 || (n % 300) == 0) Logf("createmove no client base — retry");
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
  if (!patched) {
    g_orig = nullptr;
    NoteCreateMoveToast(g_loc.slot_rva.empty() ? "no_vtable" : "no_patch", false);
    g_attempted = true;
  }
  return g_installed;
}

} // namespace cssvr
