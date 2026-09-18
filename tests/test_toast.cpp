#include "cssvrmod/toast.hpp"
#include "test_framework.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

using namespace cssvr;

TEST(toast_no_hmd_once) {
  ASSERT_TRUE(Toast_RequireOnNoHmd());
  ASSERT_FALSE(Toast_AbortVrOnFail());
  ASSERT_TRUE(Toast_IsNoHmdReason("no_hmd"));
  ASSERT_TRUE(Toast_ShouldShow("no_hmd", false));
  ASSERT_FALSE(Toast_ShouldShow("no_hmd", true));
  ASSERT_FALSE(Toast_ShouldShow("session_ok", false));
  ASSERT_FALSE(Toast_ShouldShow("idle", false));
  const auto d = Toast_Decide("no_hmd", false);
  ASSERT_TRUE(d.should_toast);
  ASSERT_FALSE(d.abort_vr);
  ASSERT_FALSE(d.path_ok);
  ASSERT_STREQ(d.risk, "no_hmd");
  ASSERT_STREQ(d.label, "XR · NO HMD");
  ASSERT_TRUE(std::strstr(d.copy, "HMD") != nullptr);
  const auto again = Toast_Decide("no_hmd", true);
  ASSERT_FALSE(again.should_toast);
  ASSERT_STREQ(again.risk, "no_hmd");
}

TEST(toast_runtime_and_silent_fail) {
  const auto loader = Toast_Decide("no_loader", false);
  ASSERT_TRUE(loader.should_toast);
  ASSERT_STREQ(loader.risk, "no_runtime");
  ASSERT_STREQ(loader.label, "XR · NO RUNTIME");
  const auto inst = Toast_Decide("create_instance", false);
  ASSERT_TRUE(inst.should_toast);
  ASSERT_TRUE(std::strstr(inst.copy, "runtime") != nullptr);
  const auto ok = Toast_Decide("session_ok", false);
  ASSERT_FALSE(ok.should_toast);
  ASSERT_TRUE(ok.path_ok);
  ASSERT_STREQ(ok.label, "XR · OK");
  const auto silent = Toast_Audit("no_hmd", false, false);
  ASSERT_TRUE(silent.should_toast);
  ASSERT_STREQ(silent.risk, "silent_fail");
  ASSERT_STREQ(silent.label, "XR · SILENT FAIL");
  ASSERT_FALSE(silent.path_ok);
  const auto honest = Toast_Audit("no_hmd", false, true);
  ASSERT_STREQ(honest.risk, "no_hmd");
}

TEST(toast_fire_writes_path) {
  char dir[] = "/tmp/cssvr_toastXXXXXX";
  ASSERT_TRUE(mkdtemp(dir) != nullptr);
  const std::string path = std::string(dir) + "/toast.txt";
  setenv("CSSVR_TOAST_PATH", path.c_str(), 1);
  setenv("CSSVR_TOAST", "0", 1);
  ASSERT_TRUE(Toast_FireDesktop(Toast_Copy("no_hmd")));
  FILE* f = std::fopen(path.c_str(), "r");
  ASSERT_TRUE(f != nullptr);
  char buf[256] = {};
  ASSERT_TRUE(std::fgets(buf, sizeof(buf), f) != nullptr);
  std::fclose(f);
  ASSERT_TRUE(std::strstr(buf, "HMD") != nullptr);
  unsetenv("CSSVR_TOAST_PATH");
  unsetenv("CSSVR_TOAST");
}
