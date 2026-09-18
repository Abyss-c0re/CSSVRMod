#pragma once
// Desktop present stamp. Fail-only — never paint on a live dual-eye submit.
#include "menu3d.hpp"
#include "toast.hpp"
#include <algorithm>
#include <cstring>

namespace cssvr {

inline bool Banner_ShouldStamp(const char* xr_reason) { return Toast_IsFailReason(xr_reason); }

inline const char* Banner_Text(const char* xr_reason) {
  if (Toast_IsNoHmdReason(xr_reason)) return "NO HMD";
  if (Toast_IsFailReason(xr_reason)) return "NO XR";
  return "";
}

inline int Banner_Height(int h) { return std::max(16, std::min(40, h / 14)); }

inline void Banner_Plot(unsigned char* px, int w, int h, bool bgra, bool flip_y, int x, int y, int r,
                        int g, int b) {
  if (!px || x < 0 || y < 0 || x >= w || y >= h) return;
  const int yy = flip_y ? (h - 1 - y) : y;
  unsigned char* p = px + ((size_t)yy * (size_t)w + (size_t)x) * 4;
  if (bgra) {
    p[0] = (unsigned char)b;
    p[1] = (unsigned char)g;
    p[2] = (unsigned char)r;
  } else {
    p[0] = (unsigned char)r;
    p[1] = (unsigned char)g;
    p[2] = (unsigned char)b;
  }
  p[3] = 255;
}

inline void Banner_Fill(unsigned char* px, int w, int h, bool bgra, bool flip_y, int x, int y, int bw,
                        int bh, int r, int g, int b) {
  for (int j = 0; j < bh; ++j)
    for (int i = 0; i < bw; ++i) Banner_Plot(px, w, h, bgra, flip_y, x + i, y + j, r, g, b);
}

inline void Banner_DrawText(unsigned char* px, int w, int h, bool bgra, bool flip_y, int x, int y,
                            int scale, const char* s, int r, int g, int b) {
  if (!px || !s || scale < 1) return;
  for (int i = 0; s[i]; ++i) {
    const uint32_t bits = Menu3d_GlyphBits(s[i]);
    for (int col = 0; col < 5; ++col)
      for (int row = 0; row < 7; ++row)
        if (bits & (1u << (row + col * 7)))
          Banner_Fill(px, w, h, bgra, flip_y, x + i * 6 * scale + col * scale, y + row * scale,
                      scale, scale, r, g, b);
  }
}

// Stamp a top bar. Returns true if pixels changed. Never stamps session_ok.
inline bool Banner_Stamp(unsigned char* px, int w, int h, bool bgra, bool flip_y,
                         const char* xr_reason) {
  if (!px || w < 32 || h < 16) return false;
  if (!Banner_ShouldStamp(xr_reason)) return false;
  const char* text = Banner_Text(xr_reason);
  if (!text || !text[0]) return false;
  const int bar = Banner_Height(h);
  Banner_Fill(px, w, h, bgra, flip_y, 0, 0, w, bar, 140, 16, 28);
  const int sc = std::max(1, (bar - 6) / 7);
  const int tw = (int)std::strlen(text) * 6 * sc;
  const int tx = std::max(4, (w - tw) / 2);
  const int ty = std::max(1, (bar - 7 * sc) / 2);
  Banner_DrawText(px, w, h, bgra, flip_y, tx, ty, sc, text, 255, 220, 80);
  return true;
}

} // namespace cssvr
