#pragma once
// Find Counter-Strike: Source and build an honest spawn plan (no Steam theater).
#include "backend.hpp"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace cssvr {

struct CssInstall {
  bool found = false;
  std::string root;
  std::string launcher; // cstrike.sh or cstrike_linux64
  std::string engine_so;
  std::string client_so;
  std::string togl_so;
  bool linux64 = false;
  const char* reason = "not_searched";
};

struct SpawnPlan {
  bool ok = false;
  std::string cwd;
  std::string exe;
  std::vector<std::string> argv;
  std::string ld_library_path;
  std::string ld_preload;
  std::string xr_runtime_json;
  std::string sdl_videodriver;
  const char* backend = "gl";
  const char* reason = "idle";
};

struct LaunchOpts {
  std::string hook_so; // libcssvrmod_hook.so
  std::string map;     // empty → menu
  int win_w = 1920; // HL2VR mirror class — 720p upscaled to HMD is potato
  int win_h = 1080;
  bool windowed = true;
  bool noborder = false; // bordered window — user-visible desktop monitor
  bool novid = true;
  bool sv_lan = true;
  Backend backend = Backend::Vk; // working present path (togl CreateDevice still dies)
  std::string extra_args;
};

struct InstallPlan {
  bool ok = false;
  std::string game_root;
  std::string hook_src;
  std::string hook_dst;
  std::string plugin_src;
  std::string plugin_dst;
  std::string vdf_dst;
  std::string cfg_dst;
  std::string steam_txt;
  const char* reason = "idle";
};

CssInstall FindCssInstall();
CssInstall InspectCssRoot(const std::string& root);
SpawnPlan PlanSpawn(const CssInstall& inst, const LaunchOpts& opts);
InstallPlan PlanInstall(const CssInstall& inst, const std::string& hook_src,
                        const std::string& plugin_src);
bool InstallToGame(const InstallPlan& p);
std::string FormatSteamLaunch(const std::string& hook_dst);
std::string DefaultHookSearchPath();
std::string DetectXrRuntimeJson();
/// Dir with libcurl-gnutls.so.4 (engine.so NEEDED). Empty if none found.
std::string DetectCssExtraLibDir();

/// Pure: never skip spawn unless hook path is empty (tests).
inline bool SpawnNeedsHook(const LaunchOpts& o) { return !o.hook_so.empty(); }

/// Pull -w/-h from spawn argv (what exec will see). False if either flag is missing.
inline bool SpawnArgvWinSize(const SpawnPlan& p, int* w, int* h) {
  if (!w || !h) return false;
  bool got_w = false, got_h = false;
  for (size_t i = 0; i + 1 < p.argv.size(); ++i) {
    if (p.argv[i] == "-w") {
      *w = std::atoi(p.argv[i + 1].c_str());
      got_w = true;
    } else if (p.argv[i] == "-h") {
      *h = std::atoi(p.argv[i + 1].c_str());
      got_h = true;
    }
  }
  return got_w && got_h;
}

/// One --print line: "-w 1280 -h 720".
inline std::string FormatSpawnWh(int w, int h) {
  char buf[48];
  std::snprintf(buf, sizeof(buf), "-w %d -h %d", w, h);
  return buf;
}

/// Prefer argv (exec truth). Fall back to planned opts when CSS is missing.
inline std::string FormatSpawnWh(const SpawnPlan& p, int fallback_w, int fallback_h) {
  int w = fallback_w, h = fallback_h;
  SpawnArgvWinSize(p, &w, &h);
  return FormatSpawnWh(w, h);
}

/// Space-joined argv for --print (no spawn).
inline std::string FormatSpawnArgv(const SpawnPlan& p) {
  std::string out;
  for (size_t i = 0; i < p.argv.size(); ++i) {
    if (i) out += ' ';
    out += p.argv[i];
  }
  return out;
}

/// Exec truth: -noborder is in argv. Empty argv (CSS missing) is not a hit.
inline bool SpawnArgvHasNoborder(const SpawnPlan& p) {
  for (const auto& a : p.argv)
    if (a == "-noborder") return true;
  return false;
}

/// --print chrome: "framed" (default law) or "-noborder" (user opt-in).
inline const char* FormatSpawnChrome(bool noborder) { return noborder ? "-noborder" : "framed"; }

/// Prefer argv. Empty argv falls back to planned opts (persisted noborder).
inline const char* FormatSpawnChrome(const SpawnPlan& p, bool fallback_noborder) {
  if (p.argv.empty()) return FormatSpawnChrome(fallback_noborder);
  return FormatSpawnChrome(SpawnArgvHasNoborder(p));
}

} // namespace cssvr
