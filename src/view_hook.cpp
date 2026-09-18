#include "cssvrmod/view_hook.hpp"
#include "cssvrmod/calib.hpp"
#include "cssvrmod/dual_paint.hpp"
#include "cssvrmod/hook_api.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/look.hpp"
#include "cssvrmod/source_if.hpp"
#include "cssvrmod/toast.hpp"
#include "cssvrmod/vk_eye.hpp"
#include "xr_host.hpp"

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

namespace cssvr {
namespace {

using RenderViewFn = void (*)(void*, void*, int, int);

RenderViewLoc g_loc;
RenderViewFn g_orig = nullptr;
EngineIf g_eng;
bool g_in = false;
bool g_did_frame = false;
GLuint g_eye[2] = {0, 0};
int g_eyeW = 0, g_eyeH = 0;
bool g_have_eyes = false;
bool g_rv_toast = false;
bool g_cap_toast = false;
int g_incomplete_n = 0;

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

void NoteLocateToast(const char* reason, bool hooked) {
  RenderViewToastIn in;
  in.locate_reason = reason;
  in.hooked = hooked;
  in.already_shown = g_rv_toast;
  const RenderViewToast t = RenderView_ToastDecide(in);
  if (!t.should_toast) return;
  g_rv_toast = true;
  Logf("cssvr toast %s %s", t.label, t.copy);
  Toast_FireDesktop(t.copy);
  Chrome_NoteStatus("NO RV");
}

void NoteCaptureToast(const DualPaintResult& r) {
  if (r.painted_dual) g_incomplete_n = 0;
  else if (r.paints == 2 && r.captures < 2) g_incomplete_n++;
  DualCaptureToastIn in;
  in.paints = r.paints;
  in.captures = r.captures;
  in.painted_dual = r.painted_dual;
  in.incomplete_frames = g_incomplete_n;
  in.already_shown = g_cap_toast;
  const DualCaptureToast t = DualCapture_ToastDecide(in);
  if (!t.should_toast) return;
  g_cap_toast = true;
  Logf("cssvr toast %s paints=%d caps=%d %s", t.label, r.paints, r.captures, t.copy);
  Toast_FireDesktop(t.copy);
  Chrome_NoteStatus("NO CAP");
}

bool ProtectWrite(void* p, bool wr) {
  const long page = sysconf(_SC_PAGESIZE);
  if (page <= 0) return false;
  auto addr = reinterpret_cast<uintptr_t>(p) & ~(uintptr_t)(page - 1);
  const int prot = PROT_READ | (wr ? PROT_WRITE : 0);
  return mprotect(reinterpret_cast<void*>(addr), (size_t)page, prot) == 0;
}

bool CopyEye(int eye) {
  if (VkCaptureEye(eye)) return true;
  GLint vp[4] = {};
  glGetIntegerv(GL_VIEWPORT, vp);
  const int w = vp[2], h = vp[3];
  if (w < 8 || h < 8) return false;
  if (!g_eye[eye] || g_eyeW != w || g_eyeH != h) {
    if (g_eye[eye]) glDeleteTextures(1, &g_eye[eye]);
    glGenTextures(1, &g_eye[eye]);
    glBindTexture(GL_TEXTURE_2D, g_eye[eye]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    g_eyeW = w;
    g_eyeH = h;
  }
  GLint draw = 0, prev = 0, read = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, draw);
  glBindTexture(GL_TEXTURE_2D, g_eye[eye]);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vp[0], vp[1], w, h);
  const GLenum err = glGetError();
  glBindTexture(GL_TEXTURE_2D, (GLuint)prev);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, read);
  return err == GL_NO_ERROR;
}

void HookedRenderView(void* self, void* view, int clear, int draw) {
  if (!g_orig) return;
  if (g_in || !view || g_did_frame) {
    g_orig(self, view, clear, draw);
    return;
  }
  constexpr size_t kBlob = 0xC0;
  int w = 0, h = 0;
  ViewSetup_ReadInt(view, kBlob, g_loc.fields.width_off, &w);
  ViewSetup_ReadInt(view, kBlob, g_loc.fields.height_off, &h);
  if (w < 640 || h < 400) {
    g_orig(self, view, clear, draw);
    return;
  }
  Vec3 origin;
  Ang3 angles;
  float fov = 90.f;
  if (!ViewSetup_ReadPose(view, kBlob, g_loc.fields, &origin, &angles, &fov)) {
    g_orig(self, view, clear, draw);
    return;
  }
  const auto look = Look_Decide(XrHostLastHmd(), nullptr, false, angles);
  if (look.applied) {
    angles = look.angles;
    ViewSetup_WriteAngles(view, kBlob, g_loc.fields, angles);
    EngineSetViewAngles(g_eng, look.angles);
  }
  g_in = true;
  StereoViewIn in;
  in.origin = origin;
  in.angles = angles;
  in.fov = fov;
  in.calib = CalibLive();
  auto r = DualPaint_Run(
      in,
      [&](const EyeView& ev) {
        ViewSetup_WriteOrigin(view, kBlob, g_loc.fields, ev.origin);
        g_orig(self, view, clear, draw);
        return true;
      },
      [&](int eye) { return CopyEye(eye); });
  ViewSetup_WriteOrigin(view, kBlob, g_loc.fields, origin);
  g_have_eyes = r.painted_dual;
  XrHostNoteDualPaint(r.painted_dual);
  g_did_frame = true;
  g_in = false;
  static int n = 0;
  if (n++ < 4 || (n % 300) == 0)
    Logf("renderview dual paints=%d caps=%d dual=%d look=%s %dx%d reason=%s", r.paints, r.captures,
         r.painted_dual ? 1 : 0, look.reason, w, h, r.reason);
  NoteCaptureToast(r);
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

void ViewHookTryInstall() {
  if (g_orig) return;
  CssInstall inst = FindCssInstall();
  if (!inst.found) {
    Logf("renderview locate skip: no css");
    NoteLocateToast("no_css", false);
    return;
  }
  if (!LocateRenderViewFile(inst.client_so.c_str(), &g_loc) || !g_loc.found) {
    Logf("renderview locate fail %s", g_loc.reason);
    NoteLocateToast(g_loc.reason ? g_loc.reason : "no_xref", false);
    return;
  }
  const uintptr_t base = ClientBase();
  if (!base) {
    Logf("renderview no client base (fn_rva=0x%llx)", (unsigned long long)g_loc.fn_rva);
    NoteLocateToast("no_client_base", false);
    return;
  }
  g_orig = reinterpret_cast<RenderViewFn>(base + g_loc.fn_rva);
  int patched = 0;
  for (uint64_t slot : g_loc.slot_rva) {
    auto* p = reinterpret_cast<void**>(base + slot);
    if (*p != reinterpret_cast<void*>(g_orig)) continue;
    if (!ProtectWrite(p, true)) continue;
    *p = reinterpret_cast<void*>(&HookedRenderView);
    ProtectWrite(p, false);
    patched++;
  }
  Logf("renderview hook fn=0x%llx slots=%d patched=%d origin=+0x%x",
       (unsigned long long)g_loc.fn_rva, (int)g_loc.slot_rva.size(), patched,
       g_loc.fields.origin_off);
  if (!patched) {
    g_orig = nullptr;
    NoteLocateToast(g_loc.slot_rva.empty() ? "fn_no_vtable" : "no_patch", false);
  }
  ProbeLiveEngine(g_eng);
  Logf("engine angles_ok=%d get=%d set=%d %s", g_eng.angles_ok ? 1 : 0, g_eng.get_angles_idx,
       g_eng.set_angles_idx, g_eng.reason);
}

void ViewHookOnSwap() { g_did_frame = false; }

bool ViewHookTakeEyes(unsigned* l, unsigned* r, int* w, int* h) {
  if (!g_have_eyes || !g_eye[0] || !g_eye[1]) return false;
  if (l) *l = g_eye[0];
  if (r) *r = g_eye[1];
  if (w) *w = g_eyeW;
  if (h) *h = g_eyeH;
  return true;
}

bool ViewHookHaveEyes() { return g_have_eyes && g_eye[0] && g_eye[1] && g_eye[0] != g_eye[1]; }

} // namespace cssvr
