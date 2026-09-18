#include "cssvrmod/usercmd.hpp"
#include <mutex>

namespace cssvr {
namespace {
UserCmdOverlay g_ov{};
bool g_have = false;
std::mutex g_ov_mu;
} // namespace

void UserCmd_NoteOverlay(const UserCmdOverlay& o) {
  std::lock_guard<std::mutex> lk(g_ov_mu);
  g_ov = o;
  g_have = true;
}

bool UserCmd_PeekOverlay(UserCmdOverlay* o) {
  if (!o) return false;
  std::lock_guard<std::mutex> lk(g_ov_mu);
  if (!g_have) return false;
  *o = g_ov;
  return true;
}

} // namespace cssvr
