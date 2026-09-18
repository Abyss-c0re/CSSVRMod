#pragma once
// Desktop CSS window must stay decorated unless the user opts into --noborder.
// SDL2 flag values match SDL_video.h.
#include <cstdint>
#include <cstring>

namespace cssvr {

constexpr uint32_t kSdlWindowFullscreen = 0x00000001u;
constexpr uint32_t kSdlWindowBorderless = 0x00000010u;
constexpr uint32_t kSdlWindowResizable = 0x00000020u;
constexpr uint32_t kSdlWindowFullscreenDesktopBit = 0x00001000u;
constexpr uint32_t kSdlWindowEvent = 0x200u;
constexpr uint8_t kSdlWindowEventResized = 5;
constexpr uint8_t kSdlWindowEventSizeChanged = 6;

inline uint32_t SanitizeSdlWindowFlags(uint32_t flags, bool allow_noborder) {
  if (allow_noborder) return flags;
  flags &= ~kSdlWindowBorderless;
  flags &= ~kSdlWindowFullscreen;
  flags &= ~kSdlWindowFullscreenDesktopBit;
  flags |= kSdlWindowResizable;
  return flags;
}

/// Parse SDL_WINDOWEVENT RESIZED / SIZE_CHANGED without linking SDL.
inline bool SdlEventWinSize(const void* ev, int ev_bytes, int* w, int* h) {
  if (!ev || ev_bytes < 24 || !w || !h) return false;
  const unsigned char* p = static_cast<const unsigned char*>(ev);
  uint32_t type = 0;
  std::memcpy(&type, p, 4);
  if (type != kSdlWindowEvent) return false;
  const uint8_t kind = p[12];
  if (kind != kSdlWindowEventResized && kind != kSdlWindowEventSizeChanged) return false;
  int32_t dw = 0, dh = 0;
  std::memcpy(&dw, p + 16, 4);
  std::memcpy(&dh, p + 20, 4);
  if (dw <= 0 || dh <= 0) return false;
  *w = (int)dw;
  *h = (int)dh;
  return true;
}

} // namespace cssvr
