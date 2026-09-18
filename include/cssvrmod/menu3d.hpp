#pragma once
// World-locked 3D settings panel (HL2VR / Cube class) — not a 2D square on the lens.
#include "calib.hpp"
#include <algorithm>
#include <cstdint>
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

inline const char* Menu3d_RowLabel(int row) {
  switch ((MenuRow)row) {
  case MenuRow::Eye: return "EYE";
  case MenuRow::Scale: return "SCALE";
  case MenuRow::HOff: return "H";
  case MenuRow::VOff: return "V";
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
  case 'N': return 0x115D671;
  case 'O': return 0x0E94A4E;
  case 'S': return 0x0E8382E;
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

inline void Menu3d_Raster(unsigned char* rgba, int w, int h, const Menu3d& m) {
  if (!rgba || w < 8 || h < 8) return;
  std::memset(rgba, 18, (size_t)w * (size_t)h * 4);
  const int titleH = std::max(20, h / 12);
  Menu3d_Fill(rgba, w, h, 0, 0, w, titleH, 140, 16, 28);
  const int scaleT = std::max(1, titleH / 10);
  Menu3d_DrawText(rgba, w, h, 8, (titleH - 7 * scaleT) / 2, scaleT, "VISION", 240, 230, 220);
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
}

} // namespace cssvr
