#pragma once
// Desktop settings host — Vision knobs + launch prefs. Not a cinema UI.
#include "backend.hpp"
#include "calib.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace cssvr {

struct Settings {
  Calib calib;
  Backend backend = Backend::Vk;
  std::string map;
  bool noborder = false;
  bool left_handed = false;
  int win_w = 1920;
  int win_h = 1080;
};

inline int Settings_ClampWin(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

/// Reject minimize / garbage. High side may still clamp (4000 → 3840).
inline bool Settings_WinSizePlausible(int w, int h) {
  return w >= 640 && h >= 480 && w <= 4096 && h <= 2304;
}

/// True if clamped size differs. Does not write the file.
inline bool Settings_NoteWinSize(Settings* s, int w, int h) {
  if (!s || !Settings_WinSizePlausible(w, h)) return false;
  const int nw = Settings_ClampWin(w, 640, 3840);
  const int nh = Settings_ClampWin(h, 480, 2160);
  if (nw == s->win_w && nh == s->win_h) return false;
  s->win_w = nw;
  s->win_h = nh;
  return true;
}

/// Debounce gate for the window hook. `force` skips the timer (destroy / unload).
inline bool Settings_ResizePersistReady(int last_w, int last_h, int w, int h, int last_ms,
                                        int now_ms, int debounce_ms, bool force, int* out_w,
                                        int* out_h) {
  if (!Settings_WinSizePlausible(w, h)) return false;
  const int nw = Settings_ClampWin(w, 640, 3840);
  const int nh = Settings_ClampWin(h, 480, 2160);
  if (out_w) *out_w = nw;
  if (out_h) *out_h = nh;
  if (nw == last_w && nh == last_h) return false;
  if (!force && debounce_ms > 0 && now_ms - last_ms < debounce_ms) return false;
  return true;
}

inline bool Settings_ApplyKey(Settings* s, const char* key, const char* val) {
  if (!s || !key || !key[0] || !val) return false;
  if (std::strcmp(key, "backend") == 0) {
    s->backend = BackendFromName(val);
    return true;
  }
  if (std::strcmp(key, "map") == 0) {
    s->map = val;
    return true;
  }
  if (std::strcmp(key, "noborder") == 0) {
    s->noborder = !(val[0] == '0' && val[1] == 0);
    return true;
  }
  if (std::strcmp(key, "left_handed") == 0 || std::strcmp(key, "lefthanded") == 0) {
    s->left_handed = !(val[0] == '0' && val[1] == 0);
    return true;
  }
  if (std::strcmp(key, "width") == 0 || std::strcmp(key, "win_w") == 0 ||
      std::strcmp(key, "w") == 0) {
    s->win_w = Settings_ClampWin(std::atoi(val), 640, 3840);
    return true;
  }
  if (std::strcmp(key, "height") == 0 || std::strcmp(key, "win_h") == 0 ||
      std::strcmp(key, "h") == 0) {
    s->win_h = Settings_ClampWin(std::atoi(val), 480, 2160);
    return true;
  }
  char buf[160];
  std::snprintf(buf, sizeof(buf), "%s %s\n", key, val);
  Calib c = s->calib;
  if (!ParseCalibText(buf, &c)) return false;
  // Unknown calib key leaves c unchanged — treat as fail if nothing matched.
  if (std::strcmp(key, "eyescale") == 0 || std::strcmp(key, "hoffset") == 0 ||
      std::strcmp(key, "horizontaloffset") == 0 || std::strcmp(key, "voffset") == 0 ||
      std::strcmp(key, "verticaloffset") == 0 || std::strcmp(key, "scalefactor") == 0 ||
      std::strcmp(key, "lens_bend") == 0 || std::strcmp(key, "lensbend") == 0 ||
      std::strcmp(key, "ipd") == 0 || std::strcmp(key, "ipd_m") == 0 ||
      std::strcmp(key, "swap_eyes") == 0 || std::strcmp(key, "swapeyes") == 0) {
    s->calib = c;
    return true;
  }
  return false;
}

inline std::string Settings_Format(const Settings& s) {
  const Calib c = ClampCalib(s.calib);
  char buf[512];
  std::snprintf(buf, sizeof(buf),
                "# CSSVRMod settings (Vision + launch)\n"
                "eyescale %.3f\n"
                "horizontaloffset %.3f\n"
                "verticaloffset %.3f\n"
                "scalefactor %.3f\n"
                "lens_bend %.3f\n"
                "ipd_m %.3f\n"
                "swap_eyes %d\n"
                "backend %s\n"
                "map %s\n"
                "noborder %d\n"
                "left_handed %d\n"
                "width %d\n"
                "height %d\n",
                c.eyescale, c.hoffset, c.voffset, c.scalefactor, c.lens_bend, c.ipd_m,
                c.swap_eyes ? 1 : 0, BackendName(s.backend), s.map.empty() ? "-" : s.map.c_str(),
                s.noborder ? 1 : 0, s.left_handed ? 1 : 0,
                Settings_ClampWin(s.win_w, 640, 3840), Settings_ClampWin(s.win_h, 480, 2160));
  return buf;
}

// Env wins. nullptr = keep existing; else "1"/"0" to export before spawn.
inline const char* Settings_LeftHandedSeed(const char* existing_env, bool cfg_left) {
  if (existing_env && existing_env[0]) return nullptr;
  return cfg_left ? "1" : "0";
}

inline bool Settings_LeftHandedFromLaunchText(const char* text) {
  Settings s;
  if (!text) return false;
  const char* p = text;
  while (*p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;
    if (*p == '#' || *p == ';' || *p == '/') {
      while (*p && *p != '\n') ++p;
      continue;
    }
    char key[64] = {}, val[64] = {};
    int ki = 0;
    while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '=' && ki < 63) key[ki++] = *p++;
    while (*p == ' ' || *p == '\t' || *p == '=') ++p;
    int vi = 0;
    while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && vi < 63) val[vi++] = *p++;
    while (*p && *p != '\n') ++p;
    if (key[0] && val[0]) Settings_ApplyKey(&s, key, val);
  }
  return s.left_handed;
}

const char* LaunchPrefsPath();
bool Settings_Load(Settings* s);
bool Settings_SaveLaunch(const Settings& s); // launch.cfg only — resize must not rewrite Vision
bool Settings_Save(const Settings& s);

/// One --print / --settings line: "launch.cfg /path ok" or "launch.cfg /path missing".
inline std::string FormatLaunchPath(const char* path, bool exists) {
  if (!path || !path[0]) return "launch.cfg -";
  char buf[560];
  std::snprintf(buf, sizeof(buf), "launch.cfg %s %s", path, exists ? "ok" : "missing");
  return buf;
}

/// --help env block. Resize persist + CSSVR_LAUNCH must be documented.
inline std::string FormatHelpEnv() {
  return "env:\n"
         "  CSSVR_LAUNCH       launch.cfg path (resize writes width/height here)\n"
         "                     default ~/.config/gvrmod/cssvr_launch.cfg\n"
         "  CSSVR_LEFT_HANDED  1=left primary; unset reads launch.cfg\n";
}

} // namespace cssvr
