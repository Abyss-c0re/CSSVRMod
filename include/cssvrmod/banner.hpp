#pragma once
// Desktop present stamp. Fail + honest MONO. Never paint on a live dual-eye submit.
#include "menu3d.hpp"
#include "toast.hpp"
#include <algorithm>
#include <cstring>

namespace cssvr {

struct BannerPlan {
  bool should_stamp = false;
  bool stamp_xr = false; // fail only — MONO stays desktop (not a cinema bar on the lenses)
  const char* text = "";
  const char* reason = "idle";
};

inline BannerPlan Banner_Decide(const char* xr_reason, bool painted_dual) {
  BannerPlan p;
  p.reason = (xr_reason && xr_reason[0]) ? xr_reason : "idle";
  if (Toast_IsNoHmdReason(p.reason)) {
    p.should_stamp = true;
    p.stamp_xr = true;
    p.text = "NO HMD";
  } else if (Toast_IsFailReason(p.reason)) {
    p.should_stamp = true;
    p.stamp_xr = true;
    p.text = "NO XR";
  } else if (!painted_dual && std::strcmp(p.reason, "session_ok") == 0) {
    p.should_stamp = true;
    p.stamp_xr = false;
    p.text = "MONO";
  }
  return p;
}

inline bool Banner_ShouldStamp(const char* xr_reason, bool painted_dual = false) {
  return Banner_Decide(xr_reason, painted_dual).should_stamp;
}

inline const char* Banner_Text(const char* xr_reason, bool painted_dual = false) {
  return Banner_Decide(xr_reason, painted_dual).text;
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

// Stamp a top bar. Returns true if pixels changed. Dual paint stays unstamped.
inline bool Banner_Stamp(unsigned char* px, int w, int h, bool bgra, bool flip_y,
                         const char* xr_reason, bool painted_dual = false) {
  if (!px || w < 32 || h < 16) return false;
  const BannerPlan plan = Banner_Decide(xr_reason, painted_dual);
  if (!plan.should_stamp || !plan.text[0]) return false;
  const int bar = Banner_Height(h);
  if (plan.stamp_xr) Banner_Fill(px, w, h, bgra, flip_y, 0, 0, w, bar, 140, 16, 28);
  else Banner_Fill(px, w, h, bgra, flip_y, 0, 0, w, bar, 36, 42, 58);
  const int sc = std::max(1, (bar - 6) / 7);
  const int tw = (int)std::strlen(plan.text) * 6 * sc;
  const int tx = std::max(4, (w - tw) / 2);
  const int ty = std::max(1, (bar - 7 * sc) / 2);
  Banner_DrawText(px, w, h, bgra, flip_y, tx, ty, sc, plan.text, 255, 220, 80);
  return true;
}

} // namespace cssvr
