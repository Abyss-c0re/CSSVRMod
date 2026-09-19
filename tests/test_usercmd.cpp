#include "cssvrmod/create_move.hpp"
#include "cssvrmod/cssvr_ctl.hpp"
#include "cssvrmod/launch.hpp"
#include "cssvrmod/usercmd.hpp"
#include "test_framework.h"
#include <cstring>
#include <vector>

using namespace cssvr;

TEST(usercmd_detect_vptr_and_apply) {
  unsigned char blob[64] = {};
  Ang3 ang{5.f, 90.f, 0.f};
  std::memcpy(blob + 16, &ang, 12);
  UserCmdFields f;
  ASSERT_TRUE(UserCmd_Detect(blob, sizeof(blob), &f));
  ASSERT_TRUE(f.has_vptr);
  ASSERT_EQ(f.fwd_off, 28);
  UserCmdOverlay o;
  o.forwardmove = 450.f;
  o.sidemove = -200.f;
  o.buttons = kInAttack | kInForward;
  o.view_pitch = 5.f;
  o.view_yaw = 90.f;
  ASSERT_TRUE(UserCmd_Apply(blob, sizeof(blob), f, o));
  float fwd = 0, side = 0;
  int buttons = 0;
  std::memcpy(&fwd, blob + 28, 4);
  std::memcpy(&side, blob + 32, 4);
  std::memcpy(&buttons, blob + 40, 4);
  ASSERT_NEAR(fwd, 450.f, 0.01);
  ASSERT_NEAR(side, -200.f, 0.01);
  ASSERT_TRUE((buttons & kInAttack) != 0);
}

TEST(usercmd_apply_no_vptr_fields) {
  unsigned char blob[64] = {};
  auto f = UserCmdFieldsNoVptr();
  UserCmdOverlay o;
  o.forwardmove = 100.f;
  o.sidemove = 50.f;
  ASSERT_TRUE(UserCmd_Apply(blob, sizeof(blob), f, o));
  float fwd = 0, side = 0;
  std::memcpy(&fwd, blob + 20, 4);
  std::memcpy(&side, blob + 24, 4);
  ASSERT_NEAR(fwd, 100.f, 0.01);
  ASSERT_NEAR(side, 50.f, 0.01);
}

TEST(clientcmd_edges_include_use_and_melee) {
  ASSERT_TRUE(ClientCmd_EdgeHas(kInAttack2));
  ASSERT_TRUE(ClientCmd_EdgeHas(kInUse));
  ASSERT_TRUE(ClientCmd_EdgeHas(kInScore));
  ASSERT_TRUE(ClientCmd_EdgeHas(kInReload));
  ASSERT_TRUE(ClientCmd_EdgeHas(kInDuck));
  ClientCmdEdge edges[16];
  const int n = ClientCmd_FillEdges(edges, 16);
  ASSERT_TRUE(n >= 11);
  bool saw_use = false, saw_duck = false, move_skipped = false, attack2_always = false;
  bool duck_always = false;
  for (int i = 0; i < n; ++i) {
    if (edges[i].bit == kInUse) saw_use = true;
    if (edges[i].bit == kInDuck) {
      saw_duck = true;
      duck_always = ClientCmd_ShouldEdge(edges[i], true);
    }
    if (edges[i].bit == kInAttack2) attack2_always = ClientCmd_ShouldEdge(edges[i], true);
    if (edges[i].analog_move) move_skipped = !ClientCmd_ShouldEdge(edges[i], true);
  }
  ASSERT_TRUE(saw_use);
  ASSERT_TRUE(saw_duck);
  ASSERT_TRUE(duck_always); // VK present must still +duck when CreateMove is live
  ASSERT_TRUE(attack2_always); // melee/use must fire even when CreateMove is live
  ASSERT_TRUE(move_skipped);
}

TEST(usercmd_note_peek) {
  UserCmdOverlay o;
  o.forwardmove = 12.f;
  UserCmd_NoteOverlay(o);
  UserCmdOverlay b{};
  ASSERT_TRUE(UserCmd_PeekOverlay(&b));
  ASSERT_NEAR(b.forwardmove, 12.f, 0.001);
}

TEST(clientcmd_release_held_clears_prev) {
  EngineIf e;
  UserCmdOverlay prev;
  prev.buttons = kInAttack | kInDuck | kInJump;
  ASSERT_TRUE(ClientCmd_ReleaseHeld(e, &prev, true));
  ASSERT_EQ(prev.buttons, 0);
  ASSERT_FALSE(ClientCmd_ReleaseHeld(e, &prev, true));
  ASSERT_FALSE(ClientCmd_ReleaseHeld(e, nullptr, true));
}

TEST(usercmd_overlay_drops_when_xr_off) {
  UserCmdOverlay o;
  o.forwardmove = 450.f;
  o.buttons = kInForward;
  UserCmd_NoteOverlay(o);
  Turn_NoteYawOff(30.f);
  CssvrSetEnabled(false);
  UserCmdOverlay b{};
  ASSERT_FALSE(UserCmd_PeekOverlay(&b));
  ASSERT_NEAR(Turn_PeekYawOff(), 0.f, 0.001);
  CssvrSetEnabled(true);
  ASSERT_TRUE(UserCmd_PeekOverlay(&b));
  ASSERT_NEAR(b.forwardmove, 450.f, 0.001);
  UserCmd_ClearOverlay();
  ASSERT_FALSE(UserCmd_PeekOverlay(&b));
  CssvrOverride().store(-1);
}

TEST(createmove_locate_css) {
  auto inst = FindCssInstall();
  if (!inst.found) return;
  CreateMoveLoc loc;
  ASSERT_TRUE(LocateCreateMoveFile(inst.client_so.c_str(), &loc));
  ASSERT_TRUE(loc.fn_rva != 0);
  ASSERT_TRUE(loc.slot >= 16 && loc.slot <= 26);
  ASSERT_TRUE(loc.slot_rva.size() >= 1);
}

TEST(createmove_locate_miss_toast) {
  CreateMoveToastIn in;
  in.locate_reason = "no_xmm0_rsi";
  auto miss = CreateMove_ToastDecide(in);
  ASSERT_TRUE(miss.should_toast);
  ASSERT_FALSE(miss.abort_vr);
  ASSERT_STREQ(miss.label, "CM · MISS");
  ASSERT_TRUE(std::strstr(miss.copy, "analog") != nullptr);
  in.already_shown = true;
  ASSERT_FALSE(CreateMove_ToastDecide(in).should_toast);
  in.already_shown = false;
  in.hooked = true;
  auto ok = CreateMove_ToastDecide(in);
  ASSERT_FALSE(ok.should_toast);
  ASSERT_STREQ(ok.reason, "hooked");
  in.hooked = false;
  in.locate_reason = "located";
  ASSERT_FALSE(CreateMove_ToastDecide(in).should_toast);
  in.locate_reason = "no_patch";
  ASSERT_TRUE(CreateMove_ToastDecide(in).should_toast);
  ASSERT_TRUE(std::strstr(CreateMove_MissCopy("no_css"), "client.so") != nullptr);
}
