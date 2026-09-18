#pragma once
// World-locked 3D settings panel (HL2VR / Cube class) — not a 2D square on the lens.
#include "calib.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace cssvr {

constexpr int kMenuRows = 5;
constexpr float kMenuW = 1.05f; // metres
constexpr float kMenuH = 0.60f;
constexpr float kMenuZ = -1.25f;
constexpr float kMenuY = 1.35f;

enum class MenuRow { Eye = 0, Scale = 1, HOff = 2, VOff = 3, Dismiss = 4 };

struct Menu3d {
  bool visible = true;
  int focus = 0;
  Calib calib;
};

inline int Menu3d_HitRow(float u, float v) {
  if (u < 0.06f || u > 0.94f || v < 0.18f || v > 0.92f) return -1;
  const int row = (int)((v - 0.18f) / ((0.92f - 0.18f) / (float)kMenuRows));
  if (row < 0 || row >= kMenuRows) return -1;
  return row;
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

inline void Menu3d_Raster(unsigned char* rgba, int w, int h, const Menu3d& m) {
  if (!rgba || w < 8 || h < 8) return;
  std::memset(rgba, 18, (size_t)w * (size_t)h * 4);
  Menu3d_Fill(rgba, w, h, 0, 0, w, 28, 140, 16, 28);
  // Title bar + rows as blocks (glyph set lives in the hook; tests check layout).
  const int rowH = (h - 40) / kMenuRows;
  for (int i = 0; i < kMenuRows; ++i) {
    const int y = 36 + i * rowH;
    const bool hot = (i == m.focus);
    Menu3d_Fill(rgba, w, h, 10, y, w - 20, rowH - 6, hot ? 160 : 36, hot ? 24 : 32, hot ? 40 : 40);
  }
  (void)m.calib;
}

} // namespace cssvr
