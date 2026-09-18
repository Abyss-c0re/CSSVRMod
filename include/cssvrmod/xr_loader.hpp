#pragma once
// Delay-load the Khronos loader. Steam pressure-vessel does not put
// libopenxr_loader.so.1 on the default search path (only the runtime under
// overrides/openxr). A DT_NEEDED on the hook means plugin_load / LD_PRELOAD
// never maps. Search sonames, then PV host + normal prefixes.
#include <cstring>

namespace cssvr {

inline const char* const* XrLoader_Candidates(int* n) {
  static const char* k[] = {
      "libopenxr_loader.so.1",
      "libopenxr_loader.so",
      "/run/host/usr/lib/libopenxr_loader.so.1",
      "/run/host/usr/lib64/libopenxr_loader.so.1",
      "/usr/lib/libopenxr_loader.so.1",
      "/usr/lib64/libopenxr_loader.so.1",
      "/usr/local/lib/libopenxr_loader.so.1",
  };
  if (n) *n = static_cast<int>(sizeof(k) / sizeof(k[0]));
  return k;
}

inline bool XrLoader_IsPvHostPath(const char* path) {
  return path && std::strstr(path, "/run/host/") && std::strstr(path, "libopenxr_loader");
}

inline bool XrLoader_HasPvHostCandidate() {
  int n = 0;
  const char* const* c = XrLoader_Candidates(&n);
  for (int i = 0; i < n; ++i)
    if (XrLoader_IsPvHostPath(c[i])) return true;
  return false;
}

/// Host loader NEEDs this. Steam PV has it only under /run/host, not cache.
inline const char* XrLoader_DepSoname() { return "libjsoncpp.so.27"; }

inline const char* const* XrLoader_DepCandidates(int* n) {
  static const char* k[] = {
      "libjsoncpp.so.27",
      "/run/host/usr/lib/libjsoncpp.so.27",
      "/run/host/usr/lib64/libjsoncpp.so.27",
      "/usr/lib/libjsoncpp.so.27",
      "/usr/lib64/libjsoncpp.so.27",
  };
  if (n) *n = static_cast<int>(sizeof(k) / sizeof(k[0]));
  return k;
}

/// Same directory as an absolute loader path. Sonames have no sibling.
inline bool XrLoader_SiblingDep(const char* loader, char* out, int n) {
  if (!loader || !out || n < 8) return false;
  out[0] = 0;
  const char* slash = std::strrchr(loader, '/');
  if (!slash || slash == loader) return false;
  const char* dep = XrLoader_DepSoname();
  const int dir_n = static_cast<int>(slash + 1 - loader);
  const int dep_n = static_cast<int>(std::strlen(dep));
  if (dir_n + dep_n + 1 > n) return false;
  std::memcpy(out, loader, static_cast<size_t>(dir_n));
  std::memcpy(out + dir_n, dep, static_cast<size_t>(dep_n) + 1);
  return true;
}

} // namespace cssvr
