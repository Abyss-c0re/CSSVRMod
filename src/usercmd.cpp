#include "cssvrmod/cssvr_ctl.hpp"
#include "cssvrmod/usercmd.hpp"
#include <atomic>
#include <mutex>

namespace cssvr {
namespace {
UserCmdOverlay g_ov{};
bool g_have = false;
float g_yaw_off = 0.f;
std::atomic<bool> g_session_ok{true};
std::mutex g_ov_mu;
} // namespace

void UserCmd_NoteOverlay(const UserCmdOverlay& o) {
  std::lock_guard<std::mutex> lk(g_ov_mu);
  g_ov = o;
  g_have = true;
}

bool UserCmd_PeekOverlay(UserCmdOverlay* o) {
  if (!o) return false;
  if (!UserCmd_OverlayLive(CssvrWantXr(), g_session_ok.load())) return false;
  std::lock_guard<std::mutex> lk(g_ov_mu);
  if (!g_have) return false;
  *o = g_ov;
  return true;
}

void UserCmd_ClearOverlay() {
  std::lock_guard<std::mutex> lk(g_ov_mu);
  g_ov = {};
  g_have = false;
  g_yaw_off = 0.f;
}

void UserCmd_NoteSessionOk(bool session_ok) {
  if (!session_ok) UserCmd_ClearOverlay();
  g_session_ok.store(session_ok);
}

void Turn_NoteYawOff(float yaw_off) {
  std::lock_guard<std::mutex> lk(g_ov_mu);
  g_yaw_off = yaw_off;
}

float Turn_PeekYawOff() {
  if (!UserCmd_OverlayLive(CssvrWantXr(), g_session_ok.load())) return 0.f;
  std::lock_guard<std::mutex> lk(g_ov_mu);
  return g_yaw_off;
}

} // namespace cssvr
