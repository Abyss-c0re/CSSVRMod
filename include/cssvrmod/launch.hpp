#pragma once
// Find Counter-Strike: Source and build an honest spawn plan (no Steam theater).
#include "backend.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
/// Best-effort: write App 240 LaunchOptions so Steam actually preloads the hook.
int InstallSteamLaunchOptions(const std::string& hook_dst);
std::string DefaultHookSearchPath();
std::string DetectXrRuntimeJson();
/// Dir with libcurl-gnutls.so.4 (engine.so NEEDED). Empty if none found.
std::string DetectCssExtraLibDir();

/// Game root from a CSS binary (`.../cstrike_linux64` / `cstrike.sh`). Empty if no slash.
inline std::string CssRootFromExe(const char* exe) {
  if (!exe || !exe[0]) return {};
  const char* slash = std::strrchr(exe, '/');
  if (!slash || slash == exe) return {};
  return std::string(exe, static_cast<size_t>(slash - exe));
}

/// Steam Cloud wipes App 240 LaunchOptions. autoexec plugin_load is the durable hook load.
inline bool Autoexec_HasPluginLoad(const char* text) {
  return text && std::strstr(text, "plugin_load") && std::strstr(text, "cssvrmod_plugin");
}

inline std::string Autoexec_WithPluginLoad(const std::string& existing) {
  if (Autoexec_HasPluginLoad(existing.c_str())) return existing;
  std::string o = existing;
  if (!o.empty() && o.back() != '\n') o += '\n';
  o += "plugin_load addons/cssvrmod/cssvrmod_plugin\n";
  return o;
}

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

inline bool SteamLaunchHasHook(const char* launch, const char* hook) {
  return launch && hook && hook[0] && std::strstr(launch, hook) != nullptr;
}

/// Keep the user's extras. Prepend LD_PRELOAD + CSSVR_XR=0 when the hook is missing.
inline std::string SteamMergeLaunchOptions(const std::string& existing, const std::string& hook_dst) {
  if (hook_dst.empty()) return existing;
  if (SteamLaunchHasHook(existing.c_str(), hook_dst.c_str())) return existing;
  const std::string ours = FormatSteamLaunch(hook_dst);
  if (existing.empty()) return ours;
  if (existing.find("%command%") != std::string::npos)
    return "LD_PRELOAD=\"" + hook_dst + "\" CSSVR_XR=0 " + existing;
  return "LD_PRELOAD=\"" + hook_dst + "\" CSSVR_XR=0 " + existing + " %command%";
}

inline std::string VdfEscape(const std::string& s) {
  std::string o;
  o.reserve(s.size() + 8);
  for (char c : s) {
    if (c == '\\' || c == '"') o.push_back('\\');
    o.push_back(c);
  }
  return o;
}

inline bool VdfParseQuoted(const std::string& s, size_t* i, std::string* out) {
  if (!i) return false;
  size_t p = *i;
  while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\r' || s[p] == '\n')) ++p;
  if (p >= s.size() || s[p] != '"') return false;
  ++p;
  std::string v;
  while (p < s.size()) {
    if (s[p] == '\\' && p + 1 < s.size()) {
      v.push_back(s[p + 1]);
      p += 2;
      continue;
    }
    if (s[p] == '"') {
      *i = p + 1;
      if (out) *out = std::move(v);
      return true;
    }
    v.push_back(s[p++]);
  }
  return false;
}

inline bool VdfMatchBrace(const std::string& s, size_t open, size_t* close) {
  if (open >= s.size() || s[open] != '{' || !close) return false;
  int depth = 0;
  for (size_t i = open; i < s.size(); ++i) {
    if (s[i] == '"') {
      size_t q = i;
      std::string dummy;
      if (!VdfParseQuoted(s, &q, &dummy)) return false;
      i = q - 1;
      continue;
    }
    if (s[i] == '{') ++depth;
    else if (s[i] == '}') {
      --depth;
      if (depth == 0) {
        *close = i;
        return true;
      }
    }
  }
  return false;
}

/// First `"appid" { ... }` object (skips `"appid" "hex"` blobs).
inline bool VdfFindAppObject(const std::string& s, const char* appid, size_t from, size_t* open,
                             size_t* close) {
  if (!appid || !appid[0] || !open || !close) return false;
  const std::string key = std::string("\"") + appid + "\"";
  size_t pos = from;
  while ((pos = s.find(key, pos)) != std::string::npos) {
    size_t p = pos + key.size();
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t' || s[p] == '\r' || s[p] == '\n')) ++p;
    if (p < s.size() && s[p] == '{') {
      size_t end = 0;
      if (!VdfMatchBrace(s, p, &end)) return false;
      *open = p;
      *close = end;
      return true;
    }
    pos = p;
  }
  return false;
}

/// Insert or replace Software/Valve/Steam/apps/<appid> LaunchOptions. First object only.
inline bool SteamUpsertAppLaunchOptions(std::string* vdf, const char* appid,
                                        const std::string& hook_dst) {
  if (!vdf || !appid || hook_dst.empty()) return false;
  size_t open = 0, close = 0;
  if (!VdfFindAppObject(*vdf, appid, 0, &open, &close)) return false;
  std::string existing;
  size_t key_at = std::string::npos, val_a = 0, val_b = 0;
  size_t i = open + 1;
  int depth = 1;
  while (i < close) {
    if ((*vdf)[i] == '"') {
      size_t ks = i;
      std::string k;
      if (!VdfParseQuoted(*vdf, &i, &k)) break;
      while (i < close && ((*vdf)[i] == ' ' || (*vdf)[i] == '\t')) ++i;
      if (i < close && (*vdf)[i] == '{') {
        size_t end = 0;
        if (!VdfMatchBrace(*vdf, i, &end)) break;
        i = end + 1;
        continue;
      }
      size_t vs = i;
      std::string val;
      if (!VdfParseQuoted(*vdf, &i, &val)) break;
      if (depth == 1 && k == "LaunchOptions") {
        key_at = ks;
        val_a = vs;
        val_b = i;
        existing = val;
        break;
      }
      continue;
    }
    if ((*vdf)[i] == '{') ++depth;
    else if ((*vdf)[i] == '}') --depth;
    ++i;
  }
  const std::string merged = SteamMergeLaunchOptions(existing, hook_dst);
  if (key_at != std::string::npos && existing == merged) return false;
  const std::string quoted = "\"" + VdfEscape(merged) + "\"";
  if (key_at != std::string::npos) {
    vdf->replace(val_a, val_b - val_a, quoted);
    return true;
  }
  std::string indent = "\t\t\t\t\t\t";
  const size_t nl = vdf->find('\n', open);
  if (nl != std::string::npos && nl < close) {
    size_t t = nl + 1;
    while (t < close && (*vdf)[t] == '\t') ++t;
    if (t > nl + 1) indent = vdf->substr(nl + 1, t - (nl + 1));
  }
  const std::string line = indent + "\"LaunchOptions\"\t\t" + quoted + "\n";
  vdf->insert(close, line);
  return true;
}

} // namespace cssvr
