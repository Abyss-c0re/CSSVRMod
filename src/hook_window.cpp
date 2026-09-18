// Force a decorated X11/SDL window. Source + DXVK often create BORDERLESS
// even when -noborder is omitted; Motif hints then hide the title bar.
#include "cssvrmod/settings.hpp"
#include "cssvrmod/window_chrome.hpp"

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <cstdlib>
#include <cstdint>
#include <time.h>

namespace {

void Log(const char* fmt, ...) {
  FILE* f = std::fopen("/tmp/cssvrmod.log", "a");
  if (!f) return;
  std::fputs("win: ", f);
  va_list ap;
  va_start(ap, fmt);
  std::vfprintf(f, fmt, ap);
  va_end(ap);
  std::fputc('\n', f);
  std::fclose(f);
}

bool AllowNoborder() {
  const char* e = std::getenv("CSSVR_NOBORDER");
  return e && e[0] == '1' && e[1] == 0;
}

using CreateWinFn = void* (*)(const char*, int, int, int, int, uint32_t);
using SetBorderFn = void (*)(void*, int);
using SetFsFn = int (*)(void*, uint32_t);
using SetReszFn = void (*)(void*, int);
using SetSizeFn = void (*)(void*, int, int);
using GetSizeFn = void (*)(void*, int*, int*);
using DestroyFn = void (*)(void*);
using PollEventFn = int (*)(void*);
using XChangePropFn = int (*)(Display*, Window, Atom, Atom, int, int, const unsigned char*, int);

extern "C" void* SDL_CreateWindow(const char*, int, int, int, int, uint32_t);
extern "C" void SDL_SetWindowBordered(void*, int);
extern "C" int SDL_SetWindowFullscreen(void*, uint32_t);
extern "C" void SDL_SetWindowSize(void*, int, int);
extern "C" void SDL_DestroyWindow(void*);
extern "C" int SDL_PollEvent(void*);
extern "C" int XChangeProperty(Display*, Window, Atom, Atom, int, int, const unsigned char*, int);

CreateWinFn g_create = nullptr;
SetBorderFn g_set_border = nullptr;
SetFsFn g_set_fs = nullptr;
SetReszFn g_set_resz = nullptr;
SetSizeFn g_set_size = nullptr;
GetSizeFn g_get_size = nullptr;
DestroyFn g_destroy = nullptr;
PollEventFn g_poll = nullptr;
XChangePropFn g_xchange = nullptr;
void* g_last_win = nullptr;
int g_creates = 0;
int g_last_w = 0;
int g_last_h = 0;
int g_last_persist_ms = 0;
using SetTitleFn = void (*)(void*, const char*);
SetTitleFn g_set_title = nullptr;
char g_last_label[32] = {};

void* OpenLib(const char* name) {
  void* h = dlopen(name, RTLD_NOW | RTLD_NOLOAD);
  if (!h) h = dlopen(name, RTLD_NOW | RTLD_LOCAL);
  return h;
}

void* SymIn(void* lib, const char* name, void* self) {
  if (!lib || !name) return nullptr;
  void* p = dlsym(lib, name);
  if (p == self) return nullptr;
  return p;
}

void EnsureSdl() {
  if (g_create && g_set_border && g_set_fs && g_set_resz) return;
  void* sdl = OpenLib("libSDL2-2.0.so.0");
  if (!sdl) sdl = OpenLib("libSDL2.so");
  if (!g_create) {
    g_create = (CreateWinFn)SymIn(sdl, "SDL_CreateWindow", (void*)SDL_CreateWindow);
    if (!g_create) g_create = (CreateWinFn)dlsym(RTLD_NEXT, "SDL_CreateWindow");
    if (g_create == (CreateWinFn)SDL_CreateWindow) g_create = nullptr;
  }
  if (!g_set_border) {
    g_set_border = (SetBorderFn)SymIn(sdl, "SDL_SetWindowBordered", (void*)SDL_SetWindowBordered);
    if (!g_set_border) g_set_border = (SetBorderFn)dlsym(RTLD_NEXT, "SDL_SetWindowBordered");
    if (g_set_border == (SetBorderFn)SDL_SetWindowBordered) g_set_border = nullptr;
  }
  if (!g_set_fs) {
    g_set_fs = (SetFsFn)SymIn(sdl, "SDL_SetWindowFullscreen", (void*)SDL_SetWindowFullscreen);
    if (!g_set_fs) g_set_fs = (SetFsFn)dlsym(RTLD_NEXT, "SDL_SetWindowFullscreen");
    if (g_set_fs == (SetFsFn)SDL_SetWindowFullscreen) g_set_fs = nullptr;
  }
  if (!g_set_resz)
    g_set_resz = (SetReszFn)(sdl ? dlsym(sdl, "SDL_SetWindowResizable")
                                 : dlsym(RTLD_NEXT, "SDL_SetWindowResizable"));
  if (!g_set_size) {
    g_set_size = (SetSizeFn)SymIn(sdl, "SDL_SetWindowSize", (void*)SDL_SetWindowSize);
    if (!g_set_size) g_set_size = (SetSizeFn)dlsym(RTLD_NEXT, "SDL_SetWindowSize");
    if (g_set_size == (SetSizeFn)SDL_SetWindowSize) g_set_size = nullptr;
  }
  if (!g_get_size)
    g_get_size = (GetSizeFn)(sdl ? dlsym(sdl, "SDL_GetWindowSize")
                                 : dlsym(RTLD_NEXT, "SDL_GetWindowSize"));
  if (!g_destroy) {
    g_destroy = (DestroyFn)SymIn(sdl, "SDL_DestroyWindow", (void*)SDL_DestroyWindow);
    if (!g_destroy) g_destroy = (DestroyFn)dlsym(RTLD_NEXT, "SDL_DestroyWindow");
    if (g_destroy == (DestroyFn)SDL_DestroyWindow) g_destroy = nullptr;
  }
  if (!g_poll) {
    g_poll = (PollEventFn)SymIn(sdl, "SDL_PollEvent", (void*)SDL_PollEvent);
    if (!g_poll) g_poll = (PollEventFn)dlsym(RTLD_NEXT, "SDL_PollEvent");
    if (g_poll == (PollEventFn)SDL_PollEvent) g_poll = nullptr;
  }
}

void EnsureX11() {
  if (g_xchange) return;
  void* x11 = OpenLib("libX11.so.6");
  if (!x11) x11 = OpenLib("libX11.so");
  g_xchange = (XChangePropFn)SymIn(x11, "XChangeProperty", (void*)XChangeProperty);
  if (!g_xchange) g_xchange = (XChangePropFn)dlsym(RTLD_NEXT, "XChangeProperty");
  if (g_xchange == (XChangePropFn)XChangeProperty) g_xchange = nullptr;
}

void ForceDecorated(void* win) {
  if (!win || AllowNoborder()) return;
  EnsureSdl();
  if (g_set_fs) g_set_fs(win, 0);
  if (g_set_border) g_set_border(win, 1);
  if (g_set_resz) g_set_resz(win, 1);
}

int NowMs() {
  timespec ts {};
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
  return (int)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void PersistWinSize(int w, int h, bool force) {
  int nw = 0, nh = 0;
  if (!cssvr::Settings_ResizePersistReady(g_last_w, g_last_h, w, h, g_last_persist_ms, NowMs(),
                                          400, force, &nw, &nh))
    return;
  cssvr::Settings s;
  if (!cssvr::Settings_Load(&s)) return;
  if (!cssvr::Settings_NoteWinSize(&s, nw, nh)) {
    g_last_w = nw;
    g_last_h = nh;
    return;
  }
  if (!cssvr::Settings_SaveLaunch(s)) return;
  g_last_w = s.win_w;
  g_last_h = s.win_h;
  g_last_persist_ms = NowMs();
  Log("persist size %dx%d", g_last_w, g_last_h);
}

} // namespace

namespace cssvr {

void Chrome_NoteStatus(const char* label) {
  if (!label || !label[0] || !g_last_win) return;
  if (g_last_label[0] && std::strcmp(g_last_label, label) == 0) return;
  std::snprintf(g_last_label, sizeof(g_last_label), "%s", label);
  EnsureSdl();
  if (!g_set_title) {
    void* sdl = OpenLib("libSDL2-2.0.so.0");
    if (!sdl) sdl = OpenLib("libSDL2.so");
    g_set_title = (SetTitleFn)(sdl ? dlsym(sdl, "SDL_SetWindowTitle")
                                   : dlsym(RTLD_NEXT, "SDL_SetWindowTitle"));
  }
  if (!g_set_title) return;
  char title[160];
  std::snprintf(title, sizeof(title), "CSSVRMod · %s", label);
  g_set_title(g_last_win, title);
  Log("title %s", title);
}

} // namespace cssvr

extern "C" {

void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, uint32_t flags) {
  EnsureSdl();
  const uint32_t raw = flags;
  if (!AllowNoborder()) flags = cssvr::SanitizeSdlWindowFlags(flags, false);
  if (!g_create) {
    Log("SDL_CreateWindow missing (no RTLD_NEXT)");
    return nullptr;
  }
  void* win = g_create(title, x, y, w, h, flags);
  g_last_win = win;
  g_creates++;
  if (cssvr::Settings_WinSizePlausible(w, h)) {
    g_last_w = cssvr::Settings_ClampWin(w, 640, 3840);
    g_last_h = cssvr::Settings_ClampWin(h, 480, 2160);
  }
  Log("SDL_CreateWindow #%d '%s' %dx%d flags 0x%x->0x%x win=%p", g_creates,
      title ? title : "", w, h, raw, flags, win);
  ForceDecorated(win);
  return win;
}

void SDL_SetWindowSize(void* window, int w, int h) {
  EnsureSdl();
  if (g_set_size) g_set_size(window, w, h);
  if (window == g_last_win) PersistWinSize(w, h, false);
}

void SDL_DestroyWindow(void* window) {
  EnsureSdl();
  if (window && window == g_last_win && g_get_size) {
    int w = 0, h = 0;
    g_get_size(window, &w, &h);
    PersistWinSize(w, h, true);
  }
  if (g_destroy) g_destroy(window);
  if (window == g_last_win) g_last_win = nullptr;
}

int SDL_PollEvent(void* event) {
  EnsureSdl();
  const int r = g_poll ? g_poll(event) : 0;
  int w = 0, h = 0;
  if (r == 1 && event && cssvr::SdlEventWinSize(event, 56, &w, &h)) PersistWinSize(w, h, false);
  return r;
}

void SDL_SetWindowBordered(void* window, int bordered) {
  EnsureSdl();
  if (!AllowNoborder()) bordered = 1;
  if (g_creates < 6) Log("SDL_SetWindowBordered %p %d", window, bordered);
  if (g_set_border) g_set_border(window, bordered);
}

int SDL_SetWindowFullscreen(void* window, uint32_t flags) {
  EnsureSdl();
  if (!AllowNoborder()) flags = 0;
  if (g_creates < 8) Log("SDL_SetWindowFullscreen %p 0x%x", window, flags);
  return g_set_fs ? g_set_fs(window, flags) : 0;
}

int XChangeProperty(Display* dpy, Window w, Atom property, Atom type, int format, int mode,
                    const unsigned char* data, int nelements) {
  EnsureX11();
  if (!g_xchange) return 0;
  if (!AllowNoborder() && dpy && property && format == 32 && data && nelements >= 3) {
    Atom motif = XInternAtom(dpy, "_MOTIF_WM_HINTS", True);
    if (motif != None && property == motif) {
      unsigned long hints[5] = {0, 0, 0, 0, 0};
      const int n = nelements < 5 ? nelements : 5;
      for (int i = 0; i < n; ++i) hints[i] = ((const unsigned long*)data)[i];
      constexpr unsigned long kDecor = 1ul << 1;
      constexpr unsigned long kDecorAll = 1ul;
      hints[0] |= kDecor;
      hints[2] = kDecorAll;
      Log("XChangeProperty motif decorations forced on win=0x%lx", (unsigned long)w);
      return g_xchange(dpy, w, property, type, format, mode,
                       reinterpret_cast<const unsigned char*>(hints), nelements);
    }
    Atom state = XInternAtom(dpy, "_NET_WM_STATE", True);
    Atom fs = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", True);
    if (state != None && fs != None && property == state && type == XA_ATOM) {
      bool stripped = false;
      unsigned long buf[16];
      int out_n = 0;
      const int n = nelements < 16 ? nelements : 16;
      for (int i = 0; i < n; ++i) {
        unsigned long a = ((const unsigned long*)data)[i];
        if (a == (unsigned long)fs) {
          stripped = true;
          continue;
        }
        buf[out_n++] = a;
      }
      if (stripped) {
        Log("XChangeProperty stripped _NET_WM_STATE_FULLSCREEN win=0x%lx", (unsigned long)w);
        return g_xchange(dpy, w, property, type, format, mode,
                         reinterpret_cast<const unsigned char*>(buf), out_n);
      }
    }
  }
  return g_xchange(dpy, w, property, type, format, mode, data, nelements);
}

} // extern "C"

__attribute__((constructor)) static void win_ctor() {
  Log("chrome hook ready force_decorated=%d", AllowNoborder() ? 0 : 1);
}

__attribute__((destructor)) static void win_dtor() {
  if (!g_last_win) return;
  EnsureSdl();
  if (!g_get_size) return;
  int w = 0, h = 0;
  g_get_size(g_last_win, &w, &h);
  PersistWinSize(w, h, true);
}
