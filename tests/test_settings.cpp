#include "cssvrmod/settings.hpp"
#include "test_framework.h"
#include <cstdlib>
#include <string>
#include <unistd.h>

using namespace cssvr;

TEST(settings_apply_vision_and_launch) {
  Settings s;
  ASSERT_TRUE(Settings_ApplyKey(&s, "eyescale", "0.20"));
  ASSERT_NEAR(s.calib.eyescale, 0.20f, 0.001);
  ASSERT_TRUE(Settings_ApplyKey(&s, "horizontaloffset", "-0.10"));
  ASSERT_NEAR(s.calib.hoffset, -0.10f, 0.001);
  ASSERT_TRUE(Settings_ApplyKey(&s, "scalefactor", "1.10"));
  ASSERT_NEAR(s.calib.scalefactor, 1.10f, 0.001);
  ASSERT_TRUE(Settings_ApplyKey(&s, "backend", "vk"));
  ASSERT_EQ((int)s.backend, (int)Backend::Vk);
  ASSERT_TRUE(Settings_ApplyKey(&s, "map", "de_dust2"));
  ASSERT_TRUE(s.map == "de_dust2");
  ASSERT_TRUE(Settings_ApplyKey(&s, "left_handed", "1"));
  ASSERT_TRUE(s.left_handed);
  ASSERT_TRUE(Settings_ApplyKey(&s, "lefthanded", "0"));
  ASSERT_FALSE(s.left_handed);
  ASSERT_TRUE(Settings_ApplyKey(&s, "width", "1280"));
  ASSERT_EQ(s.win_w, 1280);
  ASSERT_TRUE(Settings_ApplyKey(&s, "height", "720"));
  ASSERT_EQ(s.win_h, 720);
  ASSERT_TRUE(Settings_ApplyKey(&s, "width", "100"));
  ASSERT_EQ(s.win_w, 640);
  ASSERT_TRUE(Settings_ApplyKey(&s, "height", "9999"));
  ASSERT_EQ(s.win_h, 2160);
  ASSERT_FALSE(Settings_ApplyKey(&s, "cinema", "1"));
  const std::string t = Settings_Format(s);
  ASSERT_TRUE(t.find("eyescale") != std::string::npos);
  ASSERT_TRUE(t.find("backend vk") != std::string::npos);
  ASSERT_TRUE(t.find("de_dust2") != std::string::npos);
  s.left_handed = true;
  const std::string t2 = Settings_Format(s);
  ASSERT_TRUE(t2.find("left_handed 1") != std::string::npos);
  s.win_w = 1280;
  s.win_h = 720;
  const std::string t3 = Settings_Format(s);
  ASSERT_TRUE(t3.find("width 1280") != std::string::npos);
  ASSERT_TRUE(t3.find("height 720") != std::string::npos);
}

TEST(settings_seed_left_handed) {
  ASSERT_TRUE(Settings_LeftHandedFromLaunchText("backend vk\nleft_handed 1\n"));
  ASSERT_FALSE(Settings_LeftHandedFromLaunchText("left_handed 0\n"));
  ASSERT_FALSE(Settings_LeftHandedFromLaunchText("# left_handed 1\nbackend vk\n"));
  ASSERT_TRUE(Settings_LeftHandedFromLaunchText("lefthanded 1\n"));
  ASSERT_STREQ(Settings_LeftHandedSeed(nullptr, true), "1");
  ASSERT_STREQ(Settings_LeftHandedSeed("", false), "0");
  ASSERT_TRUE(Settings_LeftHandedSeed("1", false) == nullptr);
  ASSERT_TRUE(Settings_LeftHandedSeed("0", true) == nullptr);
}

TEST(settings_help_launch_env) {
  const std::string e = FormatHelpEnv();
  ASSERT_TRUE(e.find("CSSVR_LAUNCH") != std::string::npos);
  ASSERT_TRUE(e.find("cssvr_launch.cfg") != std::string::npos);
  ASSERT_TRUE(e.find("width") != std::string::npos);
  ASSERT_TRUE(e.find("resize") != std::string::npos);
  ASSERT_TRUE(e.find("CSSVR_LEFT_HANDED") != std::string::npos);
}

TEST(settings_format_launch_path) {
  ASSERT_STREQ(FormatLaunchPath(nullptr, false).c_str(), "launch.cfg -");
  ASSERT_STREQ(FormatLaunchPath("", true).c_str(), "launch.cfg -");
  const std::string ok = FormatLaunchPath("/tmp/cssvr_launch.cfg", true);
  ASSERT_TRUE(ok.find("/tmp/cssvr_launch.cfg") != std::string::npos);
  ASSERT_TRUE(ok.find(" ok") != std::string::npos);
  const std::string miss = FormatLaunchPath("/no/such/cssvr_launch.cfg", false);
  ASSERT_TRUE(miss.find(" missing") != std::string::npos);
}

TEST(settings_note_win_size_and_debounce) {
  Settings s;
  s.win_w = 1920;
  s.win_h = 1080;
  ASSERT_TRUE(Settings_NoteWinSize(&s, 1280, 720));
  ASSERT_EQ(s.win_w, 1280);
  ASSERT_EQ(s.win_h, 720);
  ASSERT_FALSE(Settings_NoteWinSize(&s, 1280, 720));
  ASSERT_FALSE(Settings_NoteWinSize(&s, 1, 1)); // minimize / garbage
  ASSERT_EQ(s.win_w, 1280);
  ASSERT_TRUE(Settings_WinSizePlausible(1280, 720));
  ASSERT_FALSE(Settings_WinSizePlausible(200, 200));
  ASSERT_TRUE(Settings_NoteWinSize(&s, 4000, 2200)); // clamp high
  ASSERT_EQ(s.win_w, 3840);
  ASSERT_EQ(s.win_h, 2160);
  int w = 0, h = 0;
  ASSERT_TRUE(Settings_ResizePersistReady(1920, 1080, 1280, 720, 0, 500, 400, false, &w, &h));
  ASSERT_EQ(w, 1280);
  ASSERT_EQ(h, 720);
  ASSERT_FALSE(Settings_ResizePersistReady(1920, 1080, 1280, 720, 0, 100, 400, false, &w, &h));
  ASSERT_TRUE(Settings_ResizePersistReady(1920, 1080, 1280, 720, 0, 100, 400, true, &w, &h));
  ASSERT_FALSE(Settings_ResizePersistReady(1280, 720, 1280, 720, 0, 1000, 400, true, &w, &h));
  ASSERT_FALSE(Settings_ResizePersistReady(1920, 1080, 32, 32, 0, 1000, 400, true, &w, &h));
}

TEST(settings_roundtrip_tmp) {
  char dir[] = "/tmp/cssvr_setXXXXXX";
  ASSERT_TRUE(mkdtemp(dir) != nullptr);
  std::string calib = std::string(dir) + "/calib.cfg";
  std::string launch = std::string(dir) + "/launch.cfg";
  setenv("CSSVR_CALIB", calib.c_str(), 1);
  setenv("CSSVR_LAUNCH", launch.c_str(), 1);
  ASSERT_STREQ(LaunchPrefsPath(), launch.c_str());
  Settings s;
  s.calib.eyescale = 0.18f;
  s.backend = Backend::Vk;
  s.map = "de_inferno";
  s.left_handed = true;
  s.win_w = 1280;
  s.win_h = 720;
  ASSERT_TRUE(Settings_Save(s));
  Settings b;
  ASSERT_TRUE(Settings_Load(&b));
  ASSERT_NEAR(b.calib.eyescale, 0.18f, 0.01);
  ASSERT_TRUE(b.map == "de_inferno");
  ASSERT_TRUE(b.left_handed);
  ASSERT_EQ(b.win_w, 1280);
  ASSERT_EQ(b.win_h, 720);
  ASSERT_TRUE(Settings_NoteWinSize(&b, 1600, 900));
  ASSERT_TRUE(Settings_SaveLaunch(b));
  Settings c;
  ASSERT_TRUE(Settings_Load(&c));
  ASSERT_EQ(c.win_w, 1600);
  ASSERT_EQ(c.win_h, 900);
  ASSERT_TRUE(c.left_handed);
  unsetenv("CSSVR_CALIB");
  unsetenv("CSSVR_LAUNCH");
}
