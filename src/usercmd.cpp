#include "cssvrmod/usercmd.hpp"

namespace cssvr {
namespace {
UserCmdOverlay g_ov{};
bool g_have = false;
} // namespace

void UserCmd_NoteOverlay(const UserCmdOverlay& o) {
  g_ov = o;
  g_have = true;
}

bool UserCmd_PeekOverlay(UserCmdOverlay* o) {
  if (!o || !g_have) return false;
  *o = g_ov;
  return true;
}

} // namespace cssvr
