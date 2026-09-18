#include "cssvrmod/settings.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace cssvr {
namespace {

std::string Home() {
  const char* h = std::getenv("HOME");
  return h ? std::string(h) : std::string();
}

void MkDirP(const std::string& dir) {
  if (dir.empty()) return;
  std::string acc;
  for (size_t i = 0; i < dir.size(); ++i) {
    acc.push_back(dir[i]);
    if (dir[i] == '/' && acc.size() > 1) mkdir(acc.c_str(), 0755);
  }
  mkdir(dir.c_str(), 0755);
}

std::string g_launch_path;

} // namespace

const char* LaunchPrefsPath() {
  if (const char* e = std::getenv("CSSVR_LAUNCH")) {
    g_launch_path = e;
    return g_launch_path.c_str();
  }
  if (g_launch_path.empty()) g_launch_path = Home() + "/.config/gvrmod/cssvr_launch.cfg";
  return g_launch_path.c_str();
}

bool Settings_Load(Settings* s) {
  if (!s) return false;
  *s = Settings{};
  CalibLive();
  s->calib = CalibLive();
  const char* path = LaunchPrefsPath();
  std::ifstream in(path);
  if (!in) return true;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#' || line[0] == ';') continue;
    std::istringstream ls(line);
    std::string k, v;
    if (!(ls >> k >> v)) continue;
    Settings_ApplyKey(s, k.c_str(), v.c_str());
  }
  return true;
}

bool Settings_Save(const Settings& s) {
  if (!CalibSave(s.calib)) return false;
  const char* path = LaunchPrefsPath();
  if (!path || !path[0]) return false;
  std::string p(path);
  auto slash = p.find_last_of('/');
  if (slash != std::string::npos) MkDirP(p.substr(0, slash));
  std::ofstream out(p);
  if (!out) return false;
  out << "backend " << BackendName(s.backend) << "\n"
      << "map " << (s.map.empty() ? "-" : s.map) << "\n"
      << "noborder " << (s.noborder ? 1 : 0) << "\n"
      << "left_handed " << (s.left_handed ? 1 : 0) << "\n"
      << "width " << Settings_ClampWin(s.win_w, 640, 3840) << "\n"
      << "height " << Settings_ClampWin(s.win_h, 480, 2160) << "\n";
  return true;
}

} // namespace cssvr
