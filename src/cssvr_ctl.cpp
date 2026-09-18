#include "cssvrmod/cssvr_ctl.hpp"
#include "cssvrmod/settings.hpp"
#include <atomic>
#include <cstdio>

namespace cssvr {
void VkLateAttachPresent();
std::atomic<int> g_menu_pulse{0};
} // namespace cssvr

namespace {
void Log(const char* msg) {
  FILE* f = std::fopen("/tmp/cssvrmod.log", "a");
  if (!f) return;
  std::fprintf(f, "cssvr: %s\n", msg);
  std::fclose(f);
}
} // namespace

extern "C" {

int CssvrEnabled() { return cssvr::CssvrWantXr() ? 1 : 0; }

void CssvrEnable(int on) {
  cssvr::CssvrSetEnabled(on != 0);
  Log(on ? "start" : "stop");
}

void CssvrShowMenu(int on) {
  cssvr::g_menu_pulse.store(on ? 1 : 2);
  Log("menu");
}

int CssvrLateAttach() {
  cssvr::VkLateAttachPresent();
  Log("late attach");
  return 1;
}

int CssvrTryConsoleLine(const char* cmd) {
  cssvr::CssvrConsole c;
  if (!cssvr::CssvrParseConsole(cmd, &c)) return 0;
  switch (c.cmd) {
  case cssvr::CssvrCmd::Start:
    cssvr::CssvrSetEnabled(true);
    Log("start");
    return 1;
  case cssvr::CssvrCmd::Stop:
    cssvr::CssvrSetEnabled(false);
    Log("stop");
    return 1;
  case cssvr::CssvrCmd::Toggle:
    cssvr::CssvrToggleEnabled();
    Log(cssvr::CssvrWantXr() ? "toggle on" : "toggle off");
    return 1;
  case cssvr::CssvrCmd::Menu:
    cssvr::g_menu_pulse.store(1);
    Log("menu");
    return 1;
  case cssvr::CssvrCmd::Set: {
    cssvr::Settings s;
    cssvr::Settings_Load(&s);
    if (c.key[0] && cssvr::Settings_ApplyKey(&s, c.key, c.val)) {
      cssvr::Settings_Save(s);
      Log("set");
    }
    return 1;
  }
  case cssvr::CssvrCmd::Status:
    Log(cssvr::CssvrWantXr() ? "status on" : "status off");
    return 1;
  case cssvr::CssvrCmd::Help:
    Log("cssvr_start | cssvr_stop | cssvr_menu | cssvr_set K V");
    return 1;
  default:
    return 0;
  }
}

} // extern "C"

namespace cssvr {

int CssvrTakeMenuPulse() { return g_menu_pulse.exchange(0); }

bool CssvrHandleEngineCmd(const char* cmd) { return CssvrTryConsoleLine(cmd) != 0; }

} // namespace cssvr
