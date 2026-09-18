#include "cssvrmod/create_move.hpp"
#include "cssvrmod/dual_paint.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/module_base.hpp"
#include "cssvrmod/view_hook.hpp"
#include "test_framework.h"
#include <cstring>
#include <vector>

using namespace cssvr;

TEST(view_setup_roundtrip_origin) {
  unsigned char blob[0x80] = {};
  ViewSetupFields f;
  Vec3 o{11.f, 22.f, 33.f};
  ASSERT_TRUE(ViewSetup_WriteOrigin(blob, sizeof(blob), f, o));
  Vec3 r;
  Ang3 a;
  float fov = 0.f;
  float fov_w = 87.5f;
  std::memcpy(blob + f.fov_off, &fov_w, 4);
  Ang3 src{1.f, 2.f, 3.f};
  std::memcpy(blob + f.angles_off, &src, 12);
  ASSERT_TRUE(ViewSetup_ReadPose(blob, sizeof(blob), f, &r, &a, &fov));
  ASSERT_NEAR(r.x, 11.f, 0.0001);
  ASSERT_NEAR(r.y, 22.f, 0.0001);
  ASSERT_NEAR(r.z, 33.f, 0.0001);
  ASSERT_NEAR(a.p, 1.f, 0.0001);
  ASSERT_NEAR(a.y, 2.f, 0.0001);
  ASSERT_NEAR(fov, 87.5f, 0.0001);
}

TEST(dual_capture_vk_live_ignores_gl_blit) {
  ASSERT_TRUE(DualCapture_Accept(true, false, true));
  ASSERT_FALSE(DualCapture_Accept(false, true, true));
  ASSERT_FALSE(DualCapture_Accept(false, false, true));
  ASSERT_TRUE(DualCapture_Accept(false, true, false));
  ASSERT_TRUE(DualCapture_Accept(true, true, false));
  ASSERT_FALSE(DualCapture_Accept(false, false, false));
}

TEST(dual_paint_gate_needs_present) {
  DualPaintFrameGate g;
  ASSERT_TRUE(DualPaint_BeginFrame(&g));
  ASSERT_FALSE(DualPaint_BeginFrame(&g));
  DualPaint_OnPresent(&g);
  ASSERT_TRUE(DualPaint_BeginFrame(&g));
  DualPaint_OnPresent(nullptr);
}

TEST(dual_paint_requires_two_captures) {
  StereoViewIn in;
  in.origin = {0.f, 0.f, 64.f};
  in.angles = {0.f, 0.f, 0.f};
  in.calib.ipd_m = 0.064f;
  in.calib.eyescale = 1.f;
  int paints = 0, caps = 0;
  auto paint = [&](const EyeView&) {
    paints++;
    return true;
  };
  auto cap_fail = [&](int) {
    caps++;
    return false;
  };
  auto r0 = DualPaint_Run(in, paint, cap_fail);
  ASSERT_EQ(r0.paints, 2);
  ASSERT_FALSE(r0.painted_dual);
  ASSERT_STREQ(r0.reason, "incomplete_dual");

  paints = caps = 0;
  auto cap_ok = [&](int) {
    caps++;
    return true;
  };
  auto r1 = DualPaint_Run(in, paint, cap_ok);
  ASSERT_TRUE(r1.painted_dual);
  ASSERT_EQ(r1.captures, 2);
  ASSERT_TRUE(r1.left_origin.y > r1.right_origin.y);
  ASSERT_NEAR(StereoView_SubmitPoseX(in.calib, 0, r1.painted_dual), 0.f, 0.0001);
}

TEST(locate_renderview_fixture_and_css) {
  std::vector<uint8_t> img(0x400, 0);
  const char* s = "CViewRender::RenderView";
  const size_t str_off = 0x300;
  std::memcpy(img.data() + str_off, s, std::strlen(s) + 1);
  const size_t fn = 0x40;
  img[fn] = 0x55;
  img[fn + 1] = 0x48;
  img[fn + 2] = 0x89;
  img[fn + 3] = 0xe5;
  img[fn + 4] = 0x49;
  img[fn + 5] = 0x89;
  img[fn + 6] = 0xf5;
  img[fn + 8] = 0xf3;
  img[fn + 9] = 0x41;
  img[fn + 10] = 0x0f;
  img[fn + 11] = 0x10;
  img[fn + 12] = 0x45;
  img[fn + 13] = 0x40;
  const size_t lea = 0x80;
  img[lea] = 0x48;
  img[lea + 1] = 0x8d;
  img[lea + 2] = 0x05;
  const int32_t disp = (int32_t)((int64_t)str_off - (int64_t)(lea + 7));
  std::memcpy(img.data() + lea + 3, &disp, 4);
  const uint64_t fn_va = (uint64_t)fn;
  std::memcpy(img.data() + 0x200, &fn_va, 8);

  RenderViewLoc loc;
  ASSERT_TRUE(LocateRenderViewInImage(img.data(), img.size(), &loc));
  ASSERT_EQ((int)loc.fn_rva, (int)fn);
  ASSERT_EQ(loc.fields.origin_off, 0x40);
  ASSERT_TRUE(loc.slot_rva.size() >= 1);

  auto inst = FindCssInstall();
  if (inst.found && !inst.client_so.empty()) {
    RenderViewLoc live;
    ASSERT_TRUE(LocateRenderViewFile(inst.client_so.c_str(), &live));
    ASSERT_TRUE(live.fn_rva != 0);
    ASSERT_EQ(live.fields.origin_off, 0x40);
    ASSERT_EQ(live.fields.angles_off, 0x4c);
    ASSERT_TRUE(live.slot_rva.size() >= 1);
  }
}

TEST(hook_install_retries_until_client_mapped) {
  ASSERT_TRUE(HookInstall_Transient("no_client_base"));
  ASSERT_FALSE(HookInstall_Transient("no_xref"));
  ASSERT_TRUE(HookInstall_ShouldRetry("no_client_base", false));
  ASSERT_TRUE(HookInstall_ShouldRetry("idle", false));
  ASSERT_FALSE(HookInstall_ShouldRetry("no_xref", false));
  ASSERT_FALSE(HookInstall_ShouldRetry("no_client_base", true));
  RenderViewToastIn in;
  in.locate_reason = "no_client_base";
  ASSERT_FALSE(RenderView_ToastDecide(in).should_toast);
  CreateMoveToastIn cm;
  cm.locate_reason = "no_client_base";
  ASSERT_FALSE(CreateMove_ToastDecide(cm).should_toast);
}

TEST(renderview_locate_miss_toast) {
  RenderViewToastIn in;
  in.locate_reason = "no_xref";
  auto miss = RenderView_ToastDecide(in);
  ASSERT_TRUE(miss.should_toast);
  ASSERT_FALSE(miss.abort_vr);
  ASSERT_STREQ(miss.label, "RV · MISS");
  ASSERT_TRUE(std::strstr(miss.copy, "MONO") != nullptr);
  in.already_shown = true;
  ASSERT_FALSE(RenderView_ToastDecide(in).should_toast);
  in.already_shown = false;
  in.hooked = true;
  auto ok = RenderView_ToastDecide(in);
  ASSERT_FALSE(ok.should_toast);
  ASSERT_STREQ(ok.reason, "hooked");
  in.hooked = false;
  in.locate_reason = "located";
  ASSERT_FALSE(RenderView_ToastDecide(in).should_toast);
  in.locate_reason = "no_patch";
  ASSERT_TRUE(RenderView_ToastDecide(in).should_toast);
  ASSERT_TRUE(std::strstr(RenderView_MissCopy("no_css"), "client.so") != nullptr);
}

TEST(dual_capture_miss_toast_after_hold) {
  DualCaptureToastIn in;
  in.paints = 2;
  in.captures = 0;
  in.incomplete_frames = 1;
  auto early = DualCapture_ToastDecide(in);
  ASSERT_FALSE(early.should_toast);
  ASSERT_FALSE(early.abort_vr);
  ASSERT_STREQ(early.reason, "incomplete_dual");
  in.incomplete_frames = DualCapture_MissHold();
  auto miss = DualCapture_ToastDecide(in);
  ASSERT_TRUE(miss.should_toast);
  ASSERT_STREQ(miss.label, "RV · NO CAP");
  ASSERT_TRUE(std::strstr(miss.copy, "MONO") != nullptr);
  in.already_shown = true;
  ASSERT_FALSE(DualCapture_ToastDecide(in).should_toast);
  in.already_shown = false;
  in.painted_dual = true;
  in.captures = 2;
  auto ok = DualCapture_ToastDecide(in);
  ASSERT_FALSE(ok.should_toast);
  ASSERT_STREQ(ok.label, "RV · 2CAP");
  in.painted_dual = false;
  in.paints = 1;
  in.captures = 0;
  in.incomplete_frames = DualCapture_MissHold();
  ASSERT_FALSE(DualCapture_ToastDecide(in).should_toast);
}

TEST(maps_module_prefers_elf_base_not_exec) {
  const char* maps =
      "7f30000000-7f30080000 r-xp 00000000 08:01 4 /home/u/.steam/steamclient.so\n"
      "7f00000000-7f00001000 r--p 00000000 08:01 1 /opt/css/cstrike/bin/linux64/client.so\n"
      "7f00001000-7f00080000 r-xp 00001000 08:01 1 /opt/css/cstrike/bin/linux64/client.so\n"
      "7f00080000-7f00090000 r--p 00080000 08:01 1 /opt/css/cstrike/bin/linux64/client.so\n"
      "7f20000000-7f20080000 r-xp 00000000 08:01 3 /opt/css/bin/linux64/engine.so\n"
      "7f10000000-7f10001000 r-xp 00000000 08:01 2 /usr/lib/libother.so\n";
  // CSS client.so first PT_LOAD is R at vaddr 0; RX is +0x644000. Hook RVAs need ELF base.
  ASSERT_EQ(Maps_ModuleBase(maps, "client.so"), (uintptr_t)0x7f00000000ull);
  ASSERT_EQ(Maps_ModuleBase(maps, "engine.so"), (uintptr_t)0x7f20000000ull);
  ASSERT_EQ(Maps_ModuleBase(maps, "nope.so"), (uintptr_t)0);
  ASSERT_EQ(Maps_ModuleBase(nullptr, "client.so"), (uintptr_t)0);
  char path[256] = {};
  ASSERT_TRUE(Maps_ModulePath(maps, "client.so", path, (int)sizeof(path)));
  ASSERT_STREQ(path, "/opt/css/cstrike/bin/linux64/client.so");
  ASSERT_TRUE(Maps_ModulePath(maps, "engine.so", path, (int)sizeof(path)));
  ASSERT_STREQ(path, "/opt/css/bin/linux64/engine.so");
  ASSERT_FALSE(Maps_ModulePath(maps, "missing.so", path, (int)sizeof(path)));

  const char* gone =
      "7f00000000-7f00001000 r--p 00000000 08:01 1 /opt/css/cstrike/bin/linux64/client.so (deleted)\n"
      "7f00001000-7f00080000 r-xp 00001000 08:01 1 /opt/css/cstrike/bin/linux64/client.so (deleted)\n";
  ASSERT_EQ(Maps_ModuleBase(gone, "client.so"), (uintptr_t)0x7f00000000ull);
  ASSERT_TRUE(Maps_ModulePath(gone, "client.so", path, (int)sizeof(path)));
  ASSERT_STREQ(path, "/opt/css/cstrike/bin/linux64/client.so");
}

TEST(module_so_plan_never_uses_main_exe) {
  const auto p = Module_SoPlan("engine.so", "/opt/css/bin/linux64/engine.so");
  ASSERT_STREQ(p.short_name, "engine.so");
  ASSERT_STREQ(p.full_path, "/opt/css/bin/linux64/engine.so");
  ASSERT_TRUE(p.try_maps);
  ASSERT_FALSE(p.try_global);
  const auto q = Module_SoPlan("", nullptr);
  ASSERT_TRUE(q.short_name == nullptr);
  ASSERT_TRUE(q.full_path == nullptr);
  ASSERT_FALSE(q.try_maps);
  ASSERT_FALSE(q.try_global);
}
