#include "cssvrmod/backend.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/source_if.hpp"
#include "cssvrmod/window_chrome.hpp"
#include "test_framework.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

using namespace cssvr;

TEST(css_root_from_in_process_exe) {
  ASSERT_TRUE(CssRootFromExe("/opt/css/cstrike_linux64") == "/opt/css");
  ASSERT_TRUE(CssRootFromExe("/opt/css/cstrike.sh") == "/opt/css");
  ASSERT_TRUE(CssRootFromExe("cstrike_linux64").empty());
  ASSERT_TRUE(CssRootFromExe(nullptr).empty());
}

TEST(launch_inspect_and_find) {
  auto missing = InspectCssRoot("/no/such/css");
  ASSERT_FALSE(missing.found);
  ASSERT_STREQ(missing.reason, "root_missing");

  auto inst = FindCssInstall();
  // Machine may or may not have CSS; both outcomes are valid.
  if (inst.found) {
    ASSERT_TRUE(inst.root.size() > 0);
    ASSERT_TRUE(inst.engine_so.find("engine.so") != std::string::npos);
    ASSERT_TRUE(inst.client_so.find("client.so") != std::string::npos);
    LaunchOpts o;
    o.hook_so.clear();
    auto plan = PlanSpawn(inst, o);
    ASSERT_TRUE(plan.ok);
    ASSERT_TRUE(plan.argv.size() >= 3);
    ASSERT_TRUE(plan.cwd == inst.root);
  } else {
    ASSERT_TRUE(inst.reason != nullptr);
  }
}

TEST(backend_gl_priority_dx9_original) {
  ASSERT_EQ(static_cast<int>(BackendFromName(nullptr)), static_cast<int>(Backend::Gl));
  ASSERT_STREQ(BackendName(Backend::Gl), "gl");
  auto gl = BackendPlan(Backend::Gl);
  ASSERT_STREQ(gl.engine_flag, "-dx9");
  ASSERT_STREQ(gl.sdl_video, "x11");
  ASSERT_STREQ(gl.hook, "gl");
  ASSERT_TRUE(BackendUsesTogl(Backend::Gl));
  auto dx = BackendPlan(Backend::Dx9);
  ASSERT_STREQ(dx.engine_flag, "-dx9");
  ASSERT_STREQ(dx.hook, "d3d9");
  ASSERT_STREQ(dx.reason, "original_vrmod_createtexture");
  auto vk = BackendPlan(Backend::Vk);
  ASSERT_STREQ(vk.engine_flag, "-vulkan");
  ASSERT_STREQ(vk.hook, "vk");
  ASSERT_FALSE(BackendUsesTogl(Backend::Vk));
}

TEST(launch_default_vk_bordered) {
  LaunchOpts o;
  ASSERT_FALSE(o.noborder);
  ASSERT_EQ(static_cast<int>(o.backend), static_cast<int>(Backend::Vk));
  CssInstall inst;
  inst.found = true;
  inst.root = "/tmp";
  inst.launcher = "/tmp/cstrike.sh";
  inst.linux64 = true;
  o.hook_so.clear();
  auto plan = PlanSpawn(inst, o);
  ASSERT_TRUE(plan.ok);
  ASSERT_STREQ(plan.backend, "vk");
  bool has_vk = false, has_noborder = false;
  for (const auto& a : plan.argv) {
    if (a == "-vulkan") has_vk = true;
    if (a == "-noborder") has_noborder = true;
  }
  ASSERT_TRUE(has_vk);
  ASSERT_FALSE(has_noborder);
  bool has_windowed = false, has_videomode = false;
  for (const auto& a : plan.argv) {
    if (a == "-windowed") has_windowed = true;
    if (a == "+mat_setvideomode") has_videomode = true;
  }
  ASSERT_TRUE(has_windowed);
  ASSERT_TRUE(has_videomode);
  o.win_w = 1280;
  o.win_h = 720;
  auto sized = PlanSpawn(inst, o);
  bool has_w = false, has_h = false;
  for (size_t i = 0; i + 1 < sized.argv.size(); ++i) {
    if (sized.argv[i] == "-w" && sized.argv[i + 1] == "1280") has_w = true;
    if (sized.argv[i] == "-h" && sized.argv[i + 1] == "720") has_h = true;
  }
  ASSERT_TRUE(has_w);
  ASSERT_TRUE(has_h);
}

TEST(launch_detects_curl_gnutls_libdir) {
  // Host may or may not have Steam sniper; env override must win when set.
  setenv("CSSVR_LIBDIR", "/tmp", 1);
  ASSERT_STREQ(DetectCssExtraLibDir().c_str(), "/tmp");
  unsetenv("CSSVR_LIBDIR");
}

TEST(sdl_event_win_size) {
  unsigned char ev[56] = {};
  const uint32_t type = kSdlWindowEvent;
  std::memcpy(ev, &type, 4);
  ev[12] = kSdlWindowEventResized;
  int32_t dw = 1280, dh = 720;
  std::memcpy(ev + 16, &dw, 4);
  std::memcpy(ev + 20, &dh, 4);
  int w = 0, h = 0;
  ASSERT_TRUE(SdlEventWinSize(ev, 56, &w, &h));
  ASSERT_EQ(w, 1280);
  ASSERT_EQ(h, 720);
  ev[12] = kSdlWindowEventSizeChanged;
  ASSERT_TRUE(SdlEventWinSize(ev, 56, &w, &h));
  ev[12] = 1; // shown
  ASSERT_FALSE(SdlEventWinSize(ev, 56, &w, &h));
  ASSERT_FALSE(SdlEventWinSize(ev, 8, &w, &h));
  ev[12] = kSdlWindowEventResized;
  uint32_t wid = 7;
  std::memcpy(ev + 8, &wid, 4);
  ASSERT_TRUE(SdlEventWinSizeFor(ev, 56, 7, &w, &h));
  ASSERT_EQ(w, 1280);
  ASSERT_FALSE(SdlEventWinSizeFor(ev, 56, 8, &w, &h)); // splash / other window
  ASSERT_FALSE(SdlEventWinSizeFor(ev, 56, 0, &w, &h)); // no last CSS window
}

TEST(sdl_window_flags_force_decorated) {
  using namespace cssvr;
  const uint32_t raw = kSdlWindowFullscreen | kSdlWindowBorderless | kSdlWindowFullscreenDesktopBit;
  const uint32_t got = SanitizeSdlWindowFlags(raw, false);
  ASSERT_EQ(got & kSdlWindowBorderless, 0u);
  ASSERT_EQ(got & kSdlWindowFullscreen, 0u);
  ASSERT_EQ(got & kSdlWindowFullscreenDesktopBit, 0u);
  ASSERT_EQ(got & kSdlWindowResizable, kSdlWindowResizable);
  ASSERT_EQ(SanitizeSdlWindowFlags(raw, true), raw);
}

TEST(launch_plan_default_gl_flag) {
  CssInstall inst;
  inst.found = true;
  inst.root = "/tmp";
  inst.launcher = "/tmp/cstrike.sh";
  inst.linux64 = true;
  LaunchOpts o;
  o.hook_so.clear();
  o.backend = Backend::Gl;
  auto plan = PlanSpawn(inst, o);
  ASSERT_TRUE(plan.ok);
  ASSERT_STREQ(plan.backend, "gl");
  ASSERT_STREQ(plan.sdl_videodriver.c_str(), "x11");
  bool has_dx9 = false;
  for (const auto& a : plan.argv)
    if (a == "-dx9") has_dx9 = true;
  ASSERT_TRUE(has_dx9);
  o.backend = Backend::Vk;
  auto pvk = PlanSpawn(inst, o);
  bool has_vk = false;
  for (const auto& a : pvk.argv)
    if (a == "-vulkan") has_vk = true;
  ASSERT_TRUE(has_vk);
}

TEST(launch_print_spawn_wh) {
  CssInstall inst;
  inst.found = true;
  inst.root = "/tmp";
  inst.launcher = "/tmp/cstrike.sh";
  inst.linux64 = true;
  LaunchOpts o;
  o.hook_so.clear();
  o.win_w = 1280;
  o.win_h = 720;
  auto plan = PlanSpawn(inst, o);
  int w = 0, h = 0;
  ASSERT_TRUE(SpawnArgvWinSize(plan, &w, &h));
  ASSERT_EQ(w, 1280);
  ASSERT_EQ(h, 720);
  const std::string line = FormatSpawnWh(plan, 1920, 1080);
  ASSERT_TRUE(line.find("-w 1280") != std::string::npos);
  ASSERT_TRUE(line.find("-h 720") != std::string::npos);
  const std::string argv = FormatSpawnArgv(plan);
  ASSERT_TRUE(argv.find("-w 1280") != std::string::npos);
  ASSERT_TRUE(argv.find("-h 720") != std::string::npos);
  ASSERT_TRUE(argv.find("+mat_setvideomode 1280 720") != std::string::npos);
  // CSS missing: --print still shows planned (persisted) size, not a blank line.
  SpawnPlan miss;
  miss.reason = "css_not_found";
  ASSERT_FALSE(SpawnArgvWinSize(miss, &w, &h));
  ASSERT_STREQ(FormatSpawnWh(miss, 1600, 900).c_str(), "-w 1600 -h 900");
  ASSERT_TRUE(FormatSpawnArgv(miss).empty());
  ASSERT_STREQ(FormatSpawnWh(1920, 1080).c_str(), "-w 1920 -h 1080");
}

TEST(launch_print_spawn_chrome) {
  CssInstall inst;
  inst.found = true;
  inst.root = "/tmp";
  inst.launcher = "/tmp/cstrike.sh";
  inst.linux64 = true;
  LaunchOpts o;
  o.hook_so.clear();
  auto framed = PlanSpawn(inst, o);
  ASSERT_FALSE(SpawnArgvHasNoborder(framed));
  ASSERT_STREQ(FormatSpawnChrome(framed, true), "framed");
  ASSERT_STREQ(FormatSpawnChrome(false), "framed");
  o.noborder = true;
  auto nb = PlanSpawn(inst, o);
  ASSERT_TRUE(SpawnArgvHasNoborder(nb));
  ASSERT_STREQ(FormatSpawnChrome(nb, false), "-noborder");
  ASSERT_STREQ(FormatSpawnChrome(true), "-noborder");
  // CSS missing: --print still shows planned chrome, not a blank / guessed framed.
  SpawnPlan miss;
  miss.reason = "css_not_found";
  ASSERT_FALSE(SpawnArgvHasNoborder(miss));
  ASSERT_STREQ(FormatSpawnChrome(miss, false), "framed");
  ASSERT_STREQ(FormatSpawnChrome(miss, true), "-noborder");
}

TEST(launch_hook_required_when_set) {
  CssInstall inst;
  inst.found = true;
  inst.root = "/tmp";
  inst.launcher = "/tmp/cstrike.sh";
  inst.linux64 = true;
  LaunchOpts o;
  o.hook_so = "/no/such/libcssvrmod_hook.so";
  auto plan = PlanSpawn(inst, o);
  ASSERT_FALSE(plan.ok);
  ASSERT_STREQ(plan.reason, "hook_missing");
  ASSERT_TRUE(SpawnNeedsHook(o));
  o.hook_so.clear();
  ASSERT_FALSE(SpawnNeedsHook(o));
}

static void* FakeFactory(const char* name, int* rc) {
  static int dummy = 1;
  if (std::strcmp(name, "VEngineClient014") == 0) {
    if (rc) *rc = 0;
    return &dummy;
  }
  if (std::strcmp(name, "VClient017") == 0) {
    if (rc) *rc = 0;
    return &dummy;
  }
  if (std::strcmp(name, "EngineTraceClient003") == 0) {
    if (rc) *rc = 0;
    return &dummy;
  }
  if (rc) *rc = 1;
  return nullptr;
}

TEST(source_if_probe_names) {
  EngineIf e;
  ASSERT_TRUE(ProbeEngineFromFactories(FakeFactory, FakeFactory, e));
  ASSERT_STREQ(e.engine_ver, "VEngineClient014");
  ASSERT_STREQ(e.client_ver, "VClient017");
  ASSERT_STREQ(e.trace_ver, "EngineTraceClient003");
  ASSERT_TRUE(e.ok);
  ASSERT_FALSE(e.screen_ok); // self-test only on live
}

TEST(engine_cmd_wraps_unrestricted) {
  ASSERT_EQ(kEngineClientCmdSlot, 7);
  ASSERT_EQ(kEngineClientCmdUnrestrictedSlot, 106);
  ASSERT_TRUE(EngineCmd_ShouldWrapSlot(7));
  ASSERT_TRUE(EngineCmd_ShouldWrapSlot(106));
  ASSERT_FALSE(EngineCmd_ShouldWrapSlot(5));
  ASSERT_FALSE(EngineCmd_ShouldWrapSlot(6));
  const auto p = EngineCmd_WrapPlan();
  ASSERT_EQ(p.n, 2);
  ASSERT_EQ(p.slots[0], 7);
  ASSERT_EQ(p.slots[1], 106);
}

TEST(engine_cmd_wrap_retries_until_unrestricted) {
  ASSERT_TRUE(EngineCmd_WrapComplete(true, true, true));
  ASSERT_TRUE(EngineCmd_WrapComplete(true, false, false));
  ASSERT_FALSE(EngineCmd_WrapComplete(false, false, false));
  ASSERT_FALSE(EngineCmd_WrapComplete(false, true, true));
  ASSERT_TRUE(EngineCmd_WrapComplete(false, true, false));
}
