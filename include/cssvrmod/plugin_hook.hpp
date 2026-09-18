#pragma once
// plugin_load late-attaches the hook. Linux RTLD_DEFAULT is NULL — that is a
// valid dlsym handle (search already-loaded objects, including LD_PRELOAD).
#include <cstring>
#include <dlfcn.h>

namespace cssvr {

inline const char* Plugin_HookSoname() { return "libcssvrmod_hook.so"; }

/// `default_search` is OpenHook's first try (RTLD_DEFAULT). A null handle is
/// ok only then. A failed RTLD_NOLOAD is a real miss.
inline bool Plugin_DlsymAllow(void* h, bool default_search) {
  if (h) return true;
  return default_search;
}

/// .../cstrike/addons/cssvrmod/<plugin> → ../../../bin/linux64/<hook>
inline bool Plugin_HookPathFromPlugin(const char* plugin_so, char* out, int n) {
  if (!out || n < 16) return false;
  out[0] = 0;
  if (!plugin_so || !plugin_so[0]) return false;
  const char* slash = std::strrchr(plugin_so, '/');
  if (!slash) return false;
  const int dir_n = static_cast<int>(slash - plugin_so);
  const char* tail = "/../../../bin/linux64/";
  const char* name = Plugin_HookSoname();
  const int need = dir_n + static_cast<int>(std::strlen(tail)) +
                   static_cast<int>(std::strlen(name)) + 1;
  if (need > n) return false;
  std::memcpy(out, plugin_so, static_cast<size_t>(dir_n));
  std::memcpy(out + dir_n, tail, std::strlen(tail) + 1);
  std::strcat(out, name);
  return true;
}

} // namespace cssvr
