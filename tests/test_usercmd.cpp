#include "cssvrmod/create_move.hpp"
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

TEST(usercmd_note_peek) {
  UserCmdOverlay o;
  o.forwardmove = 12.f;
  UserCmd_NoteOverlay(o);
  UserCmdOverlay b{};
  ASSERT_TRUE(UserCmd_PeekOverlay(&b));
  ASSERT_NEAR(b.forwardmove, 12.f, 0.001);
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
