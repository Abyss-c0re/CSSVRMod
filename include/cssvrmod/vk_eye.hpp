#pragma once
// CPU-side dual-eye store. Two distinct frames required — same buffer is heresy.
#include <cstdint>
#include <cstring>
#include <vector>

namespace cssvr {

struct VkEyeFrame {
  std::vector<unsigned char> px;
  int w = 0;
  int h = 0;
  bool bgra = false;
  bool valid = false;
};

struct VkEyePair {
  VkEyeFrame eye[2];
};

inline void VkEye_Clear(VkEyePair* p) {
  if (!p) return;
  p->eye[0] = VkEyeFrame{};
  p->eye[1] = VkEyeFrame{};
}

/// Copy pixels into one eye. Never aliases L/R — each slot owns its buffer.
inline bool VkEye_Store(VkEyePair* p, int eye, const unsigned char* px, int w, int h, bool bgra) {
  if (!p || !px || (eye != 0 && eye != 1) || w < 2 || h < 2) return false;
  const size_t n = (size_t)w * (size_t)h * 4;
  auto& e = p->eye[eye];
  e.px.assign(px, px + n);
  e.w = w;
  e.h = h;
  e.bgra = bgra;
  e.valid = true;
  return true;
}

/// Ready only when both eyes exist, same size, and buffers are distinct allocations.
inline bool VkEye_Ready(const VkEyePair& p) {
  const auto& l = p.eye[0];
  const auto& r = p.eye[1];
  if (!l.valid || !r.valid) return false;
  if (l.w != r.w || l.h != r.h || l.w < 2 || l.h < 2) return false;
  if (l.px.empty() || r.px.empty()) return false;
  if (l.px.data() == r.px.data()) return false;
  if (l.px.size() != r.px.size()) return false;
  return true;
}

/// Live: copy last Vulkan color RT into eye slot. False if no RT / copy miss.
bool VkCaptureEye(int eye);
bool VkEye_TakePair(VkEyePair* out);

} // namespace cssvr
