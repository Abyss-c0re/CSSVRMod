#pragma once
// World-locked 3D settings panel (HL2VR / Cube class) — not a 2D square on the lens.
#include "calib.hpp"
#include "vec3.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace cssvr {

constexpr int kMenuRows = 7;
constexpr float kMenuW = 1.05f; // metres
constexpr float kMenuH = 0.76f;
constexpr float kMenuZ = -1.25f;
constexpr float kMenuY = 1.35f;

enum class MenuRow { Eye = 0, Scale = 1, HOff = 2, VOff = 3, Hand = 4, Toggle = 5, Dismiss = 6 };

struct Menu3d {
  bool visible = true;
  int focus = 0;
  Calib calib;
  bool cursor = false;
  float cu = 0.5f;
  float cv = 0.5f;
  Vec3 pos{0.f, kMenuY, kMenuZ};
  float yaw = 0.f; // STAGE +Y; 0 = identity, local +Z toward +STAGE Z
  bool gripping = false;
  Vec3 grip_off{};
  bool left_handed = false;
  bool xr_on = false;
};

// Cube: laser + grip from the primary hand. 0 = left, 1 = right.
inline int Menu3d_PrimaryHand(bool left_handed) { return left_handed ? 0 : 1; }

inline float Menu3d_HandAxis(bool left_handed, float left, float right) {
  return left_handed ? left : right;
}

inline int Menu3d_HitRow(float u, float v) {
  if (u < 0.06f || u > 0.94f || v < 0.18f || v > 0.92f) return -1;
  const int row = (int)((v - 0.18f) / ((0.92f - 0.18f) / (float)kMenuRows));
  if (row < 0 || row >= kMenuRows) return -1;
  return row;
}

struct Menu3dLaserHit {
  bool hit = false;
  bool on_quad = false;
  float u = 0.f, v = 0.f, t = 0.f;
  int row = -1;
};

inline bool Menu3d_OnQuad(float u, float v) {
  return u >= 0.f && u <= 1.f && v >= 0.f && v <= 1.f;
}

inline void Menu3d_UvPx(float u, float v, int w, int h, int* x, int* y) {
  if (!x || !y || w < 1 || h < 1) return;
  const float uu = Clamp(u, 0.f, 1.f);
  const float vv = Clamp(v, 0.f, 1.f);
  *x = (int)(uu * (float)(w - 1) + 0.5f);
  *y = (int)(vv * (float)(h - 1) + 0.5f);
  if (*x < 0) *x = 0;
  if (*y < 0) *y = 0;
  if (*x >= w) *x = w - 1;
  if (*y >= h) *y = h - 1;
}

inline void Menu3d_SetCursor(Menu3d* m, const Menu3dLaserHit& h) {
  if (!m) return;
  m->cursor = h.on_quad;
  if (h.on_quad) {
    m->cu = h.u;
    m->cv = h.v;
  }
}

// STAGE metres. Identity quad at (0, kMenuY, kMenuZ) faces +Z (OpenXR quad).
inline Vec3 Menu3d_QuatRotate(float qx, float qy, float qz, float qw, const Vec3& v) {
  const Vec3 q{qx, qy, qz};
  const Vec3 t = Cross(q, v) * 2.f;
  return v + t * qw + Cross(q, t);
}

inline Vec3 Menu3d_AimFromQuat(float qx, float qy, float qz, float qw) {
  return Menu3d_QuatRotate(qx, qy, qz, qw, {0.f, 0.f, -1.f});
}

struct Menu3dQuat {
  float x = 0.f, y = 0.f, z = 0.f, w = 1.f;
};

// OpenXR quad faces local +Z. Yaw so that axis points at the HMD on the STAGE XZ plane.
inline float Menu3d_FaceYaw(const Vec3& panel, const Vec3& hmd) {
  const float dx = hmd.x - panel.x;
  const float dz = hmd.z - panel.z;
  if (dx * dx + dz * dz < 1e-6f) return 0.f;
  return std::atan2(dx, dz);
}

inline Menu3dQuat Menu3d_YawQuat(float yaw) {
  const float h = yaw * 0.5f;
  return {0.f, std::sin(h), 0.f, std::cos(h)};
}

inline Vec3 Menu3d_Normal(float yaw) { return {std::sin(yaw), 0.f, std::cos(yaw)}; }

inline Vec3 Menu3d_Right(float yaw) { return {std::cos(yaw), 0.f, -std::sin(yaw)}; }

inline void Menu3d_FaceHmd(Menu3d* m, const Vec3& hmd) {
  if (!m) return;
  m->yaw = Menu3d_FaceYaw(m->pos, hmd);
}

inline Menu3dLaserHit Menu3d_RayHit(const Vec3& origin, const Vec3& dir,
                                    const Vec3& center = Vec3{0.f, kMenuY, kMenuZ},
                                    float yaw = 0.f) {
  Menu3dLaserHit h;
  const Vec3 N = Menu3d_Normal(yaw);
  const float denom = dir.Dot(N);
  if (std::fabs(denom) < 1e-6f) return h;
  const float t = (center - origin).Dot(N) / denom;
  if (t < 0.02f || t > 8.f) return h;
  const Vec3 dlt = origin + dir * t - center;
  const Vec3 R = Menu3d_Right(yaw);
  h.t = t;
  h.u = 0.5f + dlt.Dot(R) / kMenuW;
  h.v = 0.5f - dlt.y / kMenuH;
  h.on_quad = Menu3d_OnQuad(h.u, h.v);
  h.row = Menu3d_HitRow(h.u, h.v);
  h.hit = h.row >= 0;
  return h;
}

inline void Menu3d_ClampPos(Vec3* p) {
  if (!p) return;
  p->x = Clamp(p->x, -3.f, 3.f);
  p->y = Clamp(p->y, 0.35f, 2.6f);
  p->z = Clamp(p->z, -5.f, -0.3f);
}

inline Vec3 Menu3d_DefaultPos() { return {0.f, kMenuY, kMenuZ}; }

inline bool Menu3d_TitleHit(const Menu3dLaserHit& h) {
  return h.on_quad && h.row < 0 && h.v < 0.18f;
}

inline void Menu3d_ResetPose(Menu3d* m) {
  if (!m) return;
  m->pos = Menu3d_DefaultPos();
  m->yaw = 0.f;
  m->gripping = false;
  m->grip_off = {};
}

inline bool Menu3d_OffHome(const Menu3d& m) {
  const Vec3 d = m.pos - Menu3d_DefaultPos();
  return d.LengthSqr() > 0.01f * 0.01f;
}

inline bool Menu3d_TitleHot(const Menu3d& m) { return m.cursor && m.cv < 0.18f; }

// World-locked default. Grab while the laser is on the quad starts a drag (offset = panel − hand).
inline bool Menu3d_GripTick(Menu3d* m, bool grab, const Vec3& hand, bool on_quad) {
  if (!m) return false;
  if (!grab) {
    m->gripping = false;
    return false;
  }
  if (!m->gripping) {
    if (!on_quad) return false;
    m->gripping = true;
    m->grip_off = m->pos - hand;
  }
  m->pos = hand + m->grip_off;
  Menu3d_ClampPos(&m->pos);
  return true;
}

inline bool Menu3d_ApplyClick(Menu3d* m, int row, int dir) {
  if (!m || row < 0 || row >= kMenuRows) return false;
  if (dir == 0) dir = 1;
  m->focus = row;
  Calib& c = m->calib;
  switch ((MenuRow)row) {
  case MenuRow::Eye:
    c.eyescale = CalibClamp(c.eyescale + 0.05f * (float)dir, 0.f, 1.f);
    break;
  case MenuRow::Scale:
    c.scalefactor = CalibClamp(c.scalefactor + 0.05f * (float)dir, 0.05f, 4.f);
    break;
  case MenuRow::HOff:
    c.hoffset = CalibClamp(c.hoffset + 0.05f * (float)dir, -1.f, 1.f);
    break;
  case MenuRow::VOff:
    c.voffset = CalibClamp(c.voffset + 0.05f * (float)dir, -1.f, 1.f);
    break;
  case MenuRow::Hand:
    m->left_handed = !m->left_handed;
    break;
  case MenuRow::Toggle:
    m->xr_on = !m->xr_on;
    break;
  case MenuRow::Dismiss:
    m->visible = false;
    break;
  }
  c = ClampCalib(c);
  return true;
}

// Tiny 5×7 glyph blit (same class as Cube panel — not engine VGUI).
inline void Menu3d_Plot(unsigned char* rgba, int w, int h, int x, int y, int r, int g, int b) {
  if (x < 0 || y < 0 || x >= w || y >= h) return;
  unsigned char* p = rgba + ((size_t)y * (size_t)w + (size_t)x) * 4;
  p[0] = (unsigned char)r;
  p[1] = (unsigned char)g;
  p[2] = (unsigned char)b;
  p[3] = 255;
}

inline void Menu3d_Fill(unsigned char* rgba, int w, int h, int x, int y, int bw, int bh, int r, int g,
                        int b) {
  for (int j = 0; j < bh; ++j)
    for (int i = 0; i < bw; ++i) Menu3d_Plot(rgba, w, h, x + i, y + j, r, g, b);
}

inline const char* Menu3d_RowLabel(int row) {
  switch ((MenuRow)row) {
  case MenuRow::Eye: return "EYE";
  case MenuRow::Scale: return "SCALE";
  case MenuRow::HOff: return "H";
  case MenuRow::VOff: return "V";
  case MenuRow::Hand: return "HAND";
  case MenuRow::Toggle: return "VR";
  case MenuRow::Dismiss: return "DONE";
  }
  return "";
}

inline void Menu3d_RowValue(const Menu3d& m, int row, char* out, int n) {
  if (!out || n < 2) return;
  out[0] = 0;
  const Calib c = ClampCalib(m.calib);
  switch ((MenuRow)row) {
  case MenuRow::Eye: std::snprintf(out, (size_t)n, "%.2f", c.eyescale); break;
  case MenuRow::Scale: std::snprintf(out, (size_t)n, "%.2f", c.scalefactor); break;
  case MenuRow::HOff: std::snprintf(out, (size_t)n, "%+.2f", c.hoffset); break;
  case MenuRow::VOff: std::snprintf(out, (size_t)n, "%+.2f", c.voffset); break;
  case MenuRow::Hand: std::snprintf(out, (size_t)n, "%s", m.left_handed ? "L" : "R"); break;
  case MenuRow::Toggle: std::snprintf(out, (size_t)n, "%s", m.xr_on ? "ON" : "OFF"); break;
  case MenuRow::Dismiss: std::snprintf(out, (size_t)n, "OK"); break;
  }
}

// 5×7 columns, bit0 = top. Enough for VISION labels + digits.
inline uint32_t Menu3d_GlyphBits(char ch) {
  if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 32);
  switch (ch) {
  case '0': return 0x0E9D72E;
  case '1': return 0x084210C;
  case '2': return 0x1F1322E;
  case '3': return 0x0E8722E;
  case '4': return 0x108FA98;
  case '5': return 0x0E8721F;
  case '6': return 0x0E8F22E;
  case '7': return 0x042211F;
  case '8': return 0x0E8BA2E;
  case '9': return 0x0E87A2E;
  case 'A': return 0x115F4A4;
  case 'C': return 0x0E4210E;
  case 'D': return 0x0E94A4E;
  case 'E': return 0x1E43C3E;
  case 'H': return 0x115F463;
  case 'I': return 0x0E2108E;
  case 'K': return 0x1154C63;
  case 'L': return 0x1E42108;
  case 'M': return 0xF040117F;
  case 'N': return 0x115D671;
  case 'O': return 0x0E94A4E;
  case 'R': return 0x02C244FF;
  case 'S': return 0x0E8382E;
  case 'X': return 0x32820A63;
  case 'V': return 0x0454A31;
  case 'Y': return 0x04254A5;
  case '+': return 0x0045D00;
  case '-': return 0x0005C00;
  case '.': return 0x0420000;
  case ' ': return 0;
  default: return 0x1F7BDEF;
  }
}

inline void Menu3d_DrawText(unsigned char* rgba, int w, int h, int x, int y, int scale,
                            const char* s, int r, int g, int b) {
  if (!rgba || !s || scale < 1) return;
  for (int i = 0; s[i]; ++i) {
    const uint32_t bits = Menu3d_GlyphBits(s[i]);
    for (int col = 0; col < 5; ++col)
      for (int row = 0; row < 7; ++row)
        if (bits & (1u << (row + col * 7)))
          Menu3d_Fill(rgba, w, h, x + i * 6 * scale + col * scale, y + row * scale, scale, scale, r,
                      g, b);
  }
}

// Cyan crosshair at panel UV (v=0 is the title, y-down raster). Outline so it reads on hot rows.
inline void Menu3d_DrawCursor(unsigned char* rgba, int w, int h, float u, float v) {
  if (!rgba || w < 8 || h < 8) return;
  if (!Menu3d_OnQuad(u, v)) return;
  int cx = 0, cy = 0;
  Menu3d_UvPx(u, v, w, h, &cx, &cy);
  const int arm = std::max(4, std::min(w, h) / 28);
  const int thick = 1;
  auto cross = [&](int r, int g, int b, int pad) {
    Menu3d_Fill(rgba, w, h, cx - arm - pad, cy - thick - pad, arm * 2 + 1 + pad * 2,
                thick * 2 + 1 + pad * 2, r, g, b);
    Menu3d_Fill(rgba, w, h, cx - thick - pad, cy - arm - pad, thick * 2 + 1 + pad * 2,
                arm * 2 + 1 + pad * 2, r, g, b);
  };
  cross(8, 10, 16, 1);
  cross(0, 230, 255, 0);
  Menu3d_Fill(rgba, w, h, cx - 1, cy - 1, 3, 3, 255, 255, 255);
}

inline void Menu3d_Raster(unsigned char* rgba, int w, int h, const Menu3d& m) {
  if (!rgba || w < 8 || h < 8) return;
  std::memset(rgba, 18, (size_t)w * (size_t)h * 4);
  const int titleH = std::max(20, h / 12);
  const bool titleHot = Menu3d_TitleHot(m);
  Menu3d_Fill(rgba, w, h, 0, 0, w, titleH, titleHot ? 200 : 140, titleHot ? 42 : 16,
              titleHot ? 56 : 28);
  const int scaleT = std::max(1, titleH / 10);
  Menu3d_DrawText(rgba, w, h, 8, (titleH - 7 * scaleT) / 2, scaleT, "VISION", 240, 230, 220);
  if (Menu3d_OffHome(m) && w > 96) {
    const int tw = 4 * 6 * scaleT;
    Menu3d_DrawText(rgba, w, h, w - tw - 8, (titleH - 7 * scaleT) / 2, scaleT, "HOME", 120, 220,
                    255);
  }
  const int rowH = std::max(8, (h - titleH - 8) / kMenuRows);
  for (int i = 0; i < kMenuRows; ++i) {
    const int y = titleH + 6 + i * rowH;
    const bool hot = (i == m.focus);
    Menu3d_Fill(rgba, w, h, 8, y, w - 16, rowH - 4, hot ? 160 : 36, hot ? 24 : 32, hot ? 40 : 40);
    if (rowH < 10) continue;
    const int sc = std::max(1, (rowH - 6) / 9);
    char val[16];
    Menu3d_RowValue(m, i, val, 16);
    Menu3d_DrawText(rgba, w, h, 16, y + 2, sc, Menu3d_RowLabel(i), 240, 240, 240);
    Menu3d_DrawText(rgba, w, h, w / 2, y + 2, sc, val, 255, 220, 80);
  }
  if (m.cursor) Menu3d_DrawCursor(rgba, w, h, m.cu, m.cv);
}

} // namespace cssvr
