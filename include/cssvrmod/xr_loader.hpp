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

} // namespace cssvr
