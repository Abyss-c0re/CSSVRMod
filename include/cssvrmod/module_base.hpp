#pragma once
// Resolve a path-loaded .so. dlopen("client.so", NOLOAD) misses CSS's full path.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>

namespace cssvr {

inline bool Maps_LineHasNeedle(const char* path, const char* end, const char* needle) {
  if (!path || !needle || !needle[0] || path >= end) return false;
  const size_t n = std::strlen(needle);
  for (const char* t = path; t + n <= end; ++t)
    if (std::memcmp(t, needle, n) == 0) return true;
  return false;
}

/// One /proc/self/maps line. True if the path contains needle.
inline bool Maps_ParseLine(const char* line, const char* needle, uintptr_t* start, bool* exec,
                           char* path_out, int path_n) {
  if (!line || !needle || !start) return false;
  const char* end = line;
  while (*end && *end != '\n') ++end;
  char* ep = nullptr;
  const unsigned long s = std::strtoul(line, &ep, 16);
  if (!ep || *ep != '-') return false;
  const char* p = ep + 1;
  while (p < end && *p != ' ') ++p;
  if (p < end && *p == ' ') ++p;
  const bool is_exec = (p + 2 < end && p[2] == 'x');
  int fields = 0;
  const char* q = line;
  while (q < end) {
    while (q < end && *q != ' ' && *q != '\t') ++q;
    while (q < end && (*q == ' ' || *q == '\t')) ++q;
    fields++;
    if (fields == 5) break;
  }
  if (!Maps_LineHasNeedle(q, end, needle)) return false;
  *start = (uintptr_t)s;
  if (exec) *exec = is_exec;
  if (path_out && path_n > 0) {
    int i = 0;
    while (q < end && i + 1 < path_n) path_out[i++] = *q++;
    while (i > 0 && (path_out[i - 1] == ' ' || path_out[i - 1] == '\t' || path_out[i - 1] == '\r'))
      --i;
    path_out[i] = 0;
  }
  return true;
}

/// First r-x mapping start whose path contains needle. Else first match. 0 if none.
inline uintptr_t Maps_ModuleBase(const char* maps, const char* needle) {
  if (!maps || !needle || !needle[0]) return 0;
  uintptr_t fallback = 0;
  const char* p = maps;
  while (*p) {
    uintptr_t start = 0;
    bool exec = false;
    if (Maps_ParseLine(p, needle, &start, &exec, nullptr, 0)) {
      if (exec) return start;
      if (!fallback) fallback = start;
    }
    while (*p && *p != '\n') ++p;
    if (*p == '\n') ++p;
  }
  return fallback;
}

inline bool Maps_ModulePath(const char* maps, const char* needle, char* out, int n) {
  if (!out || n < 2) return false;
  out[0] = 0;
  if (!maps || !needle) return false;
  const char* p = maps;
  char path[512];
  char saved[512] = {};
  while (*p) {
    uintptr_t start = 0;
    bool exec = false;
    if (Maps_ParseLine(p, needle, &start, &exec, path, (int)sizeof(path)) && path[0] == '/') {
      if (exec) {
        std::snprintf(out, (size_t)n, "%s", path);
        return true;
      }
      if (!saved[0]) std::snprintf(saved, sizeof(saved), "%s", path);
    }
    while (*p && *p != '\n') ++p;
    if (*p == '\n') ++p;
  }
  if (!saved[0]) return false;
  std::snprintf(out, (size_t)n, "%s", saved);
  return true;
}

inline uintptr_t Module_DladdrBase(void* handle) {
  if (!handle) return 0;
  void* ci = dlsym(handle, "CreateInterface");
  if (!ci) return 0;
  Dl_info info{};
  if (!dladdr(ci, &info) || !info.dli_fbase) return 0;
  return reinterpret_cast<uintptr_t>(info.dli_fbase);
}

/// How to open a path-loaded Source .so. Never the main executable (RTLD_DEFAULT).
struct ModuleSoPlan {
  const char* short_name = nullptr;
  const char* full_path = nullptr;
  bool try_maps = false;
  bool try_global = false;
};

inline ModuleSoPlan Module_SoPlan(const char* short_name, const char* full_path) {
  ModuleSoPlan p;
  p.short_name = (short_name && short_name[0]) ? short_name : nullptr;
  p.full_path = (full_path && full_path[0]) ? full_path : nullptr;
  p.try_maps = p.short_name != nullptr;
  p.try_global = false;
  return p;
}

/// Already-loaded handle. Short-name NOLOAD misses CSS's full path.
inline void* Module_SoHandle(const char* short_name, const char* full_path) {
  const ModuleSoPlan plan = Module_SoPlan(short_name, full_path);
  void* h = nullptr;
  if (plan.short_name) h = dlopen(plan.short_name, RTLD_NOW | RTLD_NOLOAD);
  if (!h && plan.full_path) h = dlopen(plan.full_path, RTLD_NOW | RTLD_NOLOAD);
  if (!h && plan.try_maps && plan.short_name) {
    FILE* f = std::fopen("/proc/self/maps", "r");
    if (f) {
      char line[768];
      char path[512];
      while (std::fgets(line, sizeof(line), f)) {
        uintptr_t start = 0;
        bool exec = false;
        if (!Maps_ParseLine(line, plan.short_name, &start, &exec, path, (int)sizeof(path)))
          continue;
        if (path[0] != '/') continue;
        h = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        if (h) break;
      }
      std::fclose(f);
    }
  }
  return h;
}

/// Live .so base. Maps address is last resort when NOLOAD still misses.
inline uintptr_t Module_SoBase(const char* short_name, const char* full_path) {
  void* h = Module_SoHandle(short_name, full_path);
  if (h) return Module_DladdrBase(h);
  if (!short_name || !short_name[0]) return 0;
  FILE* f = std::fopen("/proc/self/maps", "r");
  if (!f) return 0;
  char line[768];
  uintptr_t maps_base = 0;
  while (std::fgets(line, sizeof(line), f)) {
    uintptr_t start = 0;
    bool exec = false;
    if (!Maps_ParseLine(line, short_name, &start, &exec, nullptr, 0)) continue;
    if (exec) {
      maps_base = start;
      break;
    }
    if (!maps_base) maps_base = start;
  }
  std::fclose(f);
  return maps_base;
}

inline uintptr_t Module_ClientBase(const char* full_path) {
  return Module_SoBase("client.so", full_path);
}

} // namespace cssvr
