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
  ASSERT_FALSE(Settings_ApplyKey(&s, "cinema", "1"));
  const std::string t = Settings_Format(s);
  ASSERT_TRUE(t.find("eyescale") != std::string::npos);
  ASSERT_TRUE(t.find("backend vk") != std::string::npos);
  ASSERT_TRUE(t.find("de_dust2") != std::string::npos);
  s.left_handed = true;
  const std::string t2 = Settings_Format(s);
  ASSERT_TRUE(t2.find("left_handed 1") != std::string::npos);
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

TEST(settings_roundtrip_tmp) {
  char dir[] = "/tmp/cssvr_setXXXXXX";
  ASSERT_TRUE(mkdtemp(dir) != nullptr);
  std::string calib = std::string(dir) + "/calib.cfg";
  std::string launch = std::string(dir) + "/launch.cfg";
  setenv("CSSVR_CALIB", calib.c_str(), 1);
  setenv("CSSVR_LAUNCH", launch.c_str(), 1);
  Settings s;
  s.calib.eyescale = 0.18f;
  s.backend = Backend::Vk;
  s.map = "de_inferno";
  s.left_handed = true;
  ASSERT_TRUE(Settings_Save(s));
  Settings b;
  ASSERT_TRUE(Settings_Load(&b));
  ASSERT_NEAR(b.calib.eyescale, 0.18f, 0.01);
  ASSERT_TRUE(b.map == "de_inferno");
  ASSERT_TRUE(b.left_handed);
  unsetenv("CSSVR_CALIB");
  unsetenv("CSSVR_LAUNCH");
}
