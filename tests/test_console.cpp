#include "cssvrmod/console.hpp"
#include "cssvrmod/cssvr_ctl.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/menu3d.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(console_parse_start_stop_menu) {
  CssvrConsole c;
  ASSERT_TRUE(CssvrParseConsole("cssvr_start", &c));
  ASSERT_TRUE(c.cmd == CssvrCmd::Start);
  ASSERT_TRUE(CssvrParseConsole("cssvr stop", &c));
  ASSERT_TRUE(c.cmd == CssvrCmd::Stop);
  ASSERT_TRUE(CssvrParseConsole("cssvr_menu", &c));
  ASSERT_TRUE(c.cmd == CssvrCmd::Menu);
  ASSERT_TRUE(CssvrParseConsole("cssvr_set eyescale 0.20", &c));
  ASSERT_TRUE(c.cmd == CssvrCmd::Set);
  ASSERT_STREQ(c.key, "eyescale");
  ASSERT_STREQ(c.val, "0.20");
  ASSERT_FALSE(CssvrParseConsole("say hello", &c));
}

TEST(cssvr_ctl_override) {
  CssvrSetEnabled(false);
  ASSERT_FALSE(CssvrWantXr());
  CssvrSetEnabled(true);
  ASSERT_TRUE(CssvrWantXr());
  CssvrSetEnabled(false);
}

TEST(menu3d_toggle_vr) {
  Menu3d m;
  ASSERT_FALSE(m.xr_on);
  ASSERT_TRUE(Menu3d_ApplyClick(&m, (int)MenuRow::Toggle, 1));
  ASSERT_TRUE(m.xr_on);
  ASSERT_STREQ(Menu3d_RowLabel((int)MenuRow::Toggle), "VR");
}

TEST(install_plan_paths) {
  CssInstall inst;
  inst.found = true;
  inst.linux64 = true;
  inst.root = "/opt/css";
  auto p = PlanInstall(inst, "/tmp/no_such_hook.so", "");
  ASSERT_FALSE(p.ok);
  ASSERT_STREQ(p.reason, "no_hook_src");
  ASSERT_TRUE(p.hook_dst.find("bin/linux64/libcssvrmod_hook.so") != std::string::npos);
  ASSERT_TRUE(p.plugin_dst.find("cstrike/addons/cssvrmod/cssvrmod_plugin.so") != std::string::npos);
  std::string steam = FormatSteamLaunch(p.hook_dst);
  ASSERT_TRUE(steam.find("LD_PRELOAD=") != std::string::npos);
  ASSERT_TRUE(steam.find("CSSVR_XR=0") != std::string::npos);
}
