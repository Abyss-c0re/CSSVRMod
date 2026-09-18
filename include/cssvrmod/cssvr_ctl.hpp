#pragma once
// Start/stop OpenXR from the client console. Env CSSVR_XR=1 still auto-starts.
#include "console.hpp"
#include <atomic>
#include <cstdlib>
#include <cstring>

namespace cssvr {

// -1 = follow env (unset or 1 → on). 0 = forced off. 1 = forced on.
inline std::atomic<int>& CssvrOverride() {
  static std::atomic<int> g{-1};
  return g;
}

inline bool CssvrEnvWantsXr() {
  const char* e = std::getenv("CSSVR_XR");
  if (!e || !e[0]) return true; // CSSVR.sh / unset: on
  return !(e[0] == '0' && e[1] == 0);
}

inline bool CssvrWantXr() {
  const int o = CssvrOverride().load();
  if (o >= 0) return o != 0;
  return CssvrEnvWantsXr();
}

inline void CssvrSetEnabled(bool on) { CssvrOverride().store(on ? 1 : 0); }

inline void CssvrToggleEnabled() { CssvrSetEnabled(!CssvrWantXr()); }

bool CssvrHandleEngineCmd(const char* cmd);
int CssvrTakeMenuPulse();

} // namespace cssvr
