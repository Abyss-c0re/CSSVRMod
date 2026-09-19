#pragma once
// CUserCmd overlay. Analog stick → forwardmove/sidemove. Digital ClientCmd is fallback.
#include "input.hpp"
#include "source_if.hpp"
#include "vec3.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace cssvr {

struct UserCmdFields {
  int angles_off = 16; // +8 if no vptr
  int fwd_off = 28;
  int side_off = 32;
  int up_off = 36;
  int buttons_off = 40;
  bool has_vptr = true;
};

inline UserCmdFields UserCmdFieldsNoVptr() {
  UserCmdFields f;
  f.has_vptr = false;
  f.angles_off = 8;
  f.fwd_off = 20;
  f.side_off = 24;
  f.up_off = 28;
  f.buttons_off = 32;
  return f;
}

inline bool UserCmd_ReadAng(const void* cmd, size_t n, int off, Ang3* a) {
  if (!cmd || !a || off < 0 || (size_t)off + 12 > n) return false;
  std::memcpy(a, static_cast<const char*>(cmd) + off, 12);
  return true;
}

inline bool UserCmd_WriteF(void* cmd, size_t n, int off, float v) {
  if (!cmd || off < 0 || (size_t)off + 4 > n) return false;
  std::memcpy(static_cast<char*>(cmd) + off, &v, 4);
  return true;
}

inline bool UserCmd_WriteI(void* cmd, size_t n, int off, int v) {
  if (!cmd || off < 0 || (size_t)off + 4 > n) return false;
  std::memcpy(static_cast<char*>(cmd) + off, &v, 4);
  return true;
}

inline bool UserCmd_ReadI(const void* cmd, size_t n, int off, int* v) {
  if (!cmd || !v || off < 0 || (size_t)off + 4 > n) return false;
  std::memcpy(v, static_cast<const char*>(cmd) + off, 4);
  return true;
}

/// Prefer vptr layout (8CUserCmd RTTI). Fall back if viewangles at +16 are insane.
inline bool UserCmd_Detect(const void* cmd, size_t n, UserCmdFields* out) {
  if (!cmd || !out || n < 44) return false;
  UserCmdFields vptr{};
  Ang3 a{};
  if (UserCmd_ReadAng(cmd, n, vptr.angles_off, &a) && ViewAnglesSane(a)) {
    *out = vptr;
    return true;
  }
  UserCmdFields nv = UserCmdFieldsNoVptr();
  if (UserCmd_ReadAng(cmd, n, nv.angles_off, &a) && ViewAnglesSane(a)) {
    *out = nv;
    return true;
  }
  return false;
}

inline bool UserCmd_Apply(void* cmd, size_t n, const UserCmdFields& f, const UserCmdOverlay& o) {
  if (!cmd) return false;
  int buttons = 0;
  UserCmd_ReadI(cmd, n, f.buttons_off, &buttons);
  buttons |= o.buttons;
  bool ok = UserCmd_WriteF(cmd, n, f.fwd_off, o.forwardmove);
  ok = UserCmd_WriteF(cmd, n, f.side_off, o.sidemove) && ok;
  ok = UserCmd_WriteF(cmd, n, f.up_off, o.upmove) && ok;
  ok = UserCmd_WriteI(cmd, n, f.buttons_off, buttons) && ok;
  if (ViewAnglesSane({o.view_pitch, o.view_yaw, 0.f})) {
    Ang3 ang{o.view_pitch, o.view_yaw, 0.f};
    if ((size_t)f.angles_off + 12 <= n)
      std::memcpy(static_cast<char*>(cmd) + f.angles_off, &ang, 12);
  }
  return ok;
}

/// Digital ClientCmd fallback. VK present used to drop +use/+attack2/+showscores (GL had them).
struct ClientCmdEdge {
  int bit = 0;
  const char* plus = "";
  const char* minus = "";
  bool analog_move = false;
};

inline int ClientCmd_FillEdges(ClientCmdEdge* out, int n) {
  static const ClientCmdEdge k[] = {
      {kInAttack, "+attack", "-attack", false},
      {kInAttack2, "+attack2", "-attack2", false},
      {kInJump, "+jump", "-jump", false},
      {kInDuck, "+duck", "-duck", false},
      {kInReload, "+reload", "-reload", false},
      {kInUse, "+use", "-use", false},
      {kInScore, "+showscores", "-showscores", false},
      {kInForward, "+forward", "-forward", true},
      {kInBack, "+back", "-back", true},
      {kInMoveLeft, "+moveleft", "-moveleft", true},
      {kInMoveRight, "+moveright", "-moveright", true},
  };
  const int m = (int)(sizeof(k) / sizeof(k[0]));
  if (!out || n <= 0) return m;
  int w = 0;
  for (; w < m && w < n; ++w) out[w] = k[w];
  return m;
}

inline bool ClientCmd_ShouldEdge(const ClientCmdEdge& e, bool usercmd_live) {
  return !e.analog_move || !usercmd_live;
}

inline bool ClientCmd_EdgeHas(int bit) {
  ClientCmdEdge edges[16];
  const int n = ClientCmd_FillEdges(edges, 16);
  for (int i = 0; i < n; ++i)
    if (edges[i].bit == bit) return true;
  return false;
}

inline void ClientCmd_ApplyEdges(const EngineIf& eng, const UserCmdOverlay& cmd,
                                 const UserCmdOverlay& prev, bool usercmd_live) {
  ClientCmdEdge edges[16];
  const int n = ClientCmd_FillEdges(edges, 16);
  for (int i = 0; i < n; ++i) {
    if (!ClientCmd_ShouldEdge(edges[i], usercmd_live)) continue;
    const bool now = (cmd.buttons & edges[i].bit) != 0;
    const bool was = (prev.buttons & edges[i].bit) != 0;
    if (now && !was) EngineClientCmd(eng, edges[i].plus);
    if (!now && was) EngineClientCmd(eng, edges[i].minus);
  }
  if (cmd.lastinv && !prev.lastinv) EngineClientCmd(eng, "lastinv");
}

void UserCmd_NoteOverlay(const UserCmdOverlay& o);
bool UserCmd_PeekOverlay(UserCmdOverlay* o);
void UserCmd_ClearOverlay();
void Turn_NoteYawOff(float yaw_off);
float Turn_PeekYawOff();
bool UserCmd_HookLive();

} // namespace cssvr
