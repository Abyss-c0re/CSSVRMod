#include "cssvrmod/input.hpp"
#include "test_framework.h"

using namespace cssvr;

TEST(input_map_fire_and_move) {
  XrSample xr;
  xr.hmd.valid = true;
  xr.hmd.ang = {0, 45, 0};
  xr.right.valid = true;
  xr.right.ang = {0, 90, 0};
  xr.trigger_r = 0.9f;
  xr.stick_ly = 1.f;
  xr.a_click = true;
  GunPose gun;
  gun.valid = true;
  gun.forward = {0, 1, 0};
  InputConfig cfg;
  auto cmd = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_TRUE(cmd.firing);
  ASSERT_TRUE((cmd.buttons & kInAttack) != 0);
  ASSERT_TRUE((cmd.buttons & kInJump) != 0);
  ASSERT_TRUE((cmd.buttons & kInForward) != 0);
  ASSERT_TRUE(cmd.look_from_gun);
  ASSERT_NEAR(cmd.view_yaw, 90.f, 2.f);
  ASSERT_STREQ(cmd.reason, "aim_gun");

  xr.trigger_r = 0.f;
  xr.trigger_l = 0.9f;
  cfg.left_handed = true;
  auto lefty = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_TRUE(lefty.firing);
  cfg.left_handed = false;
  auto idle = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_FALSE(idle.firing);
  ASSERT_FALSE(idle.look_from_gun);
  ASSERT_NEAR(idle.view_yaw, 45.f, 2.f);
}

TEST(input_stick_right_turns_right) {
  XrSample xr;
  xr.hmd.valid = true;
  xr.hmd.ang = {0, 0, 0};
  GunPose gun;
  InputConfig cfg;
  cfg.smooth_turn = true;
  cfg.snap_turn = false;
  cfg.turn_speed = 90.f;
  xr.stick_rx = 1.f;
  auto smooth = InputMap(xr, gun, cfg, 1.f);
  ASSERT_TRUE(smooth.view_yaw < -1.f);

  cfg.snap_turn = true;
  cfg.snap_yaw = 30.f;
  auto snap_r = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_NEAR(snap_r.view_yaw, -30.f, 0.1f);
  xr.stick_rx = -1.f;
  auto snap_l = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_NEAR(snap_l.view_yaw, 30.f, 0.1f);
}

TEST(input_smooth_turn_accumulates) {
  XrSample xr;
  xr.hmd.valid = true;
  xr.hmd.ang = {0, 0, 0};
  xr.stick_rx = 1.f;
  GunPose gun;
  InputConfig cfg;
  cfg.smooth_turn = true;
  cfg.snap_turn = false;
  cfg.turn_speed = 90.f;
  TurnState t;
  auto a = InputMap(xr, gun, cfg, 1.f, &t);
  auto b = InputMap(xr, gun, cfg, 1.f, &t);
  ASSERT_NEAR(a.view_yaw, -90.f, 1.f);
  ASSERT_NEAR(b.view_yaw, -180.f, 1.f);
}

TEST(input_snap_turn_latches) {
  XrSample xr;
  xr.hmd.valid = true;
  xr.stick_rx = 1.f;
  GunPose gun;
  InputConfig cfg;
  cfg.snap_turn = true;
  cfg.snap_yaw = 30.f;
  TurnState t;
  auto a = InputMap(xr, gun, cfg, 0.01f, &t);
  auto b = InputMap(xr, gun, cfg, 0.01f, &t);
  ASSERT_NEAR(a.view_yaw, -30.f, 0.1f);
  ASSERT_NEAR(b.view_yaw, -30.f, 0.1f); // still held — one snap
  xr.stick_rx = 0.f;
  InputMap(xr, gun, cfg, 0.01f, &t);
  xr.stick_rx = 1.f;
  auto c = InputMap(xr, gun, cfg, 0.01f, &t);
  ASSERT_NEAR(c.view_yaw, -60.f, 0.1f);
}

TEST(input_y_click_lastinv) {
  XrSample xr;
  xr.y_click = true;
  xr.b_click = true;
  xr.x_click = true;
  GunPose gun;
  InputConfig cfg;
  auto cmd = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_TRUE(cmd.lastinv);
  ASSERT_TRUE((cmd.buttons & kInReload) != 0);
  ASSERT_TRUE((cmd.buttons & kInUse) != 0);
  xr.y_click = false;
  xr.b_click = false;
  xr.x_click = false;
  auto idle = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_FALSE(idle.lastinv);
}

TEST(input_stick_click_ducks) {
  XrSample xr;
  xr.stick_click_l = true;
  GunPose gun;
  InputConfig cfg;
  auto cmd = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_TRUE((cmd.buttons & kInDuck) != 0);
  ASSERT_TRUE((cmd.buttons & kInJump) == 0);
  auto p = PlanSdlInject(cmd, Ang3{0, 0, 0}, 0.022f);
  ASSERT_TRUE(p.key_ctrl);
  xr.stick_click_l = false;
  auto idle = InputMap(xr, gun, cfg, 0.01f);
  ASSERT_TRUE((idle.buttons & kInDuck) == 0);
}

TEST(input_deadzone_and_sdl_plan) {
  ASSERT_NEAR(ApplyDead(0.05f, 0.18f), 0.f, 1e-6);
  ASSERT_TRUE(ApplyDead(1.f, 0.18f) > 0.9f);
  UserCmdOverlay cmd;
  cmd.view_yaw = 30.f;
  cmd.buttons = kInAttack | kInForward;
  Ang3 cur{0, 0, 0};
  auto p = PlanSdlInject(cmd, cur, 0.022f);
  ASSERT_TRUE(p.mouse_left);
  ASSERT_TRUE(p.key_w);
  ASSERT_TRUE(p.mouse_dx > 0.f);
}
