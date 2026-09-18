#include "cssvrmod/console.hpp"
#include "cssvrmod/cssvr_ctl.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/menu3d.hpp"
#include "cssvrmod/plugin_hook.hpp"
#include "cssvrmod/source_if.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(console_decode_cbuf_jmp) {
  unsigned char buf[16] = {0x48, 0x89, 0xf7, 0xe9, 0x10, 0x00, 0x00, 0x00};
  void* t = EngineCmd_DecodeCbuf(buf);
  ASSERT_TRUE(t == (void*)(buf + 8 + 0x10));
}

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
  ASSERT_TRUE(CssvrParseConsole("cssvr_start\n", &c));
  ASSERT_TRUE(c.cmd == CssvrCmd::Start);
  ASSERT_TRUE(CssvrParseConsole("cssvr_start\r\n", &c));
  ASSERT_TRUE(c.cmd == CssvrCmd::Start);
  ASSERT_TRUE(CssvrParseConsole("  cssvr stop\n", &c));
  ASSERT_TRUE(c.cmd == CssvrCmd::Stop);
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

TEST(plugin_bind_allows_rtld_default) {
  ASSERT_TRUE(Plugin_DlsymAllow(RTLD_DEFAULT, true));
  ASSERT_FALSE(Plugin_DlsymAllow(nullptr, false));
  ASSERT_TRUE(Plugin_DlsymAllow(reinterpret_cast<void*>(1), false));
  ASSERT_STREQ(Plugin_HookSoname(), "libcssvrmod_hook.so");
  char path[256];
  ASSERT_TRUE(Plugin_HookPathFromPlugin("/opt/css/cstrike/addons/cssvrmod/cssvrmod_plugin.so",
                                        path, 256));
  ASSERT_TRUE(std::strstr(path, "/opt/css/cstrike/addons/cssvrmod/../../../bin/linux64/") !=
              nullptr);
  ASSERT_TRUE(std::strstr(path, "libcssvrmod_hook.so") != nullptr);
  ASSERT_FALSE(Plugin_HookPathFromPlugin("cssvrmod_plugin.so", path, 256));
  ASSERT_FALSE(Plugin_HookPathFromPlugin(nullptr, path, 256));
}

TEST(autoexec_plugin_load_once) {
  ASSERT_FALSE(Autoexec_HasPluginLoad(""));
  ASSERT_FALSE(Autoexec_HasPluginLoad("echo hi\n"));
  std::string a = Autoexec_WithPluginLoad("");
  ASSERT_TRUE(Autoexec_HasPluginLoad(a.c_str()));
  ASSERT_TRUE(a.find("plugin_load addons/cssvrmod/cssvrmod_plugin") != std::string::npos);
  ASSERT_TRUE(Autoexec_WithPluginLoad(a) == a);
  std::string keep = Autoexec_WithPluginLoad("cl_showfps 1");
  ASSERT_TRUE(keep.find("cl_showfps 1") != std::string::npos);
  ASSERT_TRUE(Autoexec_HasPluginLoad(keep.c_str()));
}

TEST(steam_merge_launch_options) {
  const char* hook = "/opt/css/bin/linux64/libcssvrmod_hook.so";
  ASSERT_FALSE(SteamLaunchHasHook("", hook));
  std::string empty = SteamMergeLaunchOptions("", hook);
  ASSERT_TRUE(SteamLaunchHasHook(empty.c_str(), hook));
  ASSERT_TRUE(empty.find("CSSVR_XR=0") != std::string::npos);
  ASSERT_TRUE(empty.find("%command%") != std::string::npos);
  std::string again = SteamMergeLaunchOptions(empty, hook);
  ASSERT_TRUE(again == empty);
  std::string keep = SteamMergeLaunchOptions("PRESSURE_VESSEL_IMPORT_OPENXR_1_RUNTIMES=1 %command%", hook);
  ASSERT_TRUE(SteamLaunchHasHook(keep.c_str(), hook));
  ASSERT_TRUE(keep.find("PRESSURE_VESSEL_IMPORT_OPENXR_1_RUNTIMES=1") != std::string::npos);
  // Symlink spelling: do not stack a second LD_PRELOAD.
  const char* alt = "/home/u/.local/share/Steam/steamapps/common/Counter-Strike Source/bin/linux64/libcssvrmod_hook.so";
  const char* via_steam =
      "LD_PRELOAD=\"/home/u/.steam/steam/steamapps/common/Counter-Strike Source/bin/linux64/libcssvrmod_hook.so\" CSSVR_XR=0 %command%";
  ASSERT_TRUE(SteamLaunchHasHook(via_steam, alt));
  ASSERT_TRUE(SteamMergeLaunchOptions(via_steam, alt) == via_steam);
}

TEST(steam_upsert_app_240_launch_options) {
  const char* hook = "/opt/css/bin/linux64/libcssvrmod_hook.so";
  std::string vdf =
      "\"Software\"\n{\n\t\"Valve\"\n\t{\n\t\t\"Steam\"\n\t\t{\n\t\t\t\"apps\"\n\t\t\t{\n"
      "\t\t\t\t\"240\"\t\t\"deadbeef\"\n"
      "\t\t\t\t\"240\"\n\t\t\t\t{\n\t\t\t\t\t\"LastPlayed\"\t\t\"1\"\n"
      "\t\t\t\t\t\"Playtime\"\t\t\"2\"\n\t\t\t\t}\n"
      "\t\t\t\t\"4000\"\n\t\t\t\t{\n\t\t\t\t\t\"LaunchOptions\"\t\t\"keep-me\"\n\t\t\t\t}\n"
      "\t\t\t}\n\t\t}\n\t}\n}\n";
  ASSERT_TRUE(SteamUpsertAppLaunchOptions(&vdf, "240", hook));
  ASSERT_TRUE(vdf.find("deadbeef") != std::string::npos);
  ASSERT_TRUE(vdf.find("keep-me") != std::string::npos);
  ASSERT_TRUE(SteamLaunchHasHook(vdf.c_str(), hook));
  ASSERT_FALSE(SteamUpsertAppLaunchOptions(&vdf, "240", hook));
  std::string other = vdf;
  ASSERT_TRUE(SteamUpsertAppLaunchOptions(&other, "4000", hook));
  ASSERT_TRUE(SteamLaunchHasHook(other.c_str(), hook));
  ASSERT_TRUE(other.find("keep-me") != std::string::npos);
}
