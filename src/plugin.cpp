// Source server plugin — plugin_load addons/cssvrmod/cssvrmod_plugin
// Loads the hook if Steam was started without LD_PRELOAD, then late-attaches present.
#include "cssvrmod/plugin_hook.hpp"
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <string>

namespace {

using CreateInterfaceFn = void* (*)(const char*, int*);
using LateFn = int (*)();
using EnableFn = void (*)(int);
using TryFn = int (*)(const char*);

LateFn g_late = nullptr;
EnableFn g_enable = nullptr;
TryFn g_try = nullptr;
void* g_hook = nullptr;

void Log(const char* m) {
  FILE* f = std::fopen("/tmp/cssvrmod.log", "a");
  if (!f) return;
  std::fprintf(f, "plugin: %s\n", m);
  std::fclose(f);
}

bool BindHook(void* h, bool default_search) {
  if (!cssvr::Plugin_DlsymAllow(h, default_search)) return false;
  g_late = (LateFn)dlsym(h, "CssvrLateAttach");
  g_enable = (EnableFn)dlsym(h, "CssvrEnable");
  g_try = (TryFn)dlsym(h, "CssvrTryConsoleLine");
  return g_late != nullptr;
}

bool OpenHook() {
  if (BindHook(RTLD_DEFAULT, true)) return true;
  void* loaded = dlopen(cssvr::Plugin_HookSoname(), RTLD_NOW | RTLD_NOLOAD);
  if (BindHook(loaded, false)) {
    g_hook = loaded;
    return true;
  }
  char path[512];
  std::string hook = cssvr::Plugin_HookSoname();
  Dl_info info{};
  if (dladdr((void*)&OpenHook, &info) && info.dli_fname &&
      cssvr::Plugin_HookPathFromPlugin(info.dli_fname, path, (int)sizeof(path)))
    hook = path;
  g_hook = dlopen(hook.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!g_hook) {
    Log(dlerror() ? dlerror() : "dlopen hook failed");
    return false;
  }
  return BindHook(g_hook, false);
}

class Plugin {
 public:
  virtual bool Load(CreateInterfaceFn, CreateInterfaceFn) {
    Log("load");
    if (!OpenHook()) return false;
    if (g_late) g_late();
    return true;
  }
  virtual void Unload() { Log("unload"); }
  virtual void Pause() {}
  virtual void UnPause() {}
  virtual const char* GetPluginDescription() { return "CSSVRMod"; }
  virtual void LevelInit(const char*) {}
  virtual void ServerActivate(void*, int, int) {}
  virtual void GameFrame(bool) {}
  virtual void LevelShutdown() {}
  virtual void ClientActive(void*) {}
  virtual void ClientDisconnect(void*) {}
  virtual void ClientPutInServer(void*, const char*) {}
  virtual void SetCommandClient(int) {}
  virtual void ClientSettingsChanged(void*) {}
  virtual int ClientConnect(bool*, void*, const char*, const char*, char*, int) { return 0; }
  virtual int ClientCommand(void*, const void*) { return 0; }
  virtual int NetworkIDValidated(const char*, const char*) { return 0; }
  virtual void OnQueryCvarValueFinished(int, void*, int, const char*, const char*) {}
  virtual void OnEdictAllocated(void*) {}
  virtual void OnEdictFreed(const void*) {}
};

Plugin g_plugin;

} // namespace

extern "C" void* CreateInterface(const char* name, int* ret) {
  if (name && std::strstr(name, "ISERVERPLUGINCALLBACKS")) {
    if (ret) *ret = 0;
    return &g_plugin;
  }
  if (ret) *ret = 1;
  return nullptr;
}
