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

// Color RTs must be copyable. shaderapivk / DXVK often omit TRANSFER_SRC.
constexpr uint32_t kVkUsageTransferSrc = 0x00000001u;
constexpr uint32_t kVkUsageColorAtt = 0x00000010u;

inline uint32_t VkEye_AugmentUsage(uint32_t usage) {
  if (usage & kVkUsageColorAtt) usage |= kVkUsageTransferSrc;
  return usage;
}

struct VkEyeMissIn {
  bool have_img = false;
  uint32_t w = 0, h = 0;
  bool have_dev = false;
  bool have_queue = false;
  bool copied = false;
};

inline const char* VkEye_MissDecide(const VkEyeMissIn& in) {
  if (!in.have_img) return "no_rt";
  if (in.w < 8 || in.h < 8) return "tiny";
  if (!in.have_dev) return "no_dev";
  if (!in.have_queue) return "no_queue";
  if (!in.copied) return "copy_fail";
  return "ok";
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

/// Same pixels twice is one world view. Pose IPD on that pair is heresy.
inline bool VkEye_WorldsDiffer(const VkEyePair& p) {
  if (!VkEye_Ready(p)) return false;
  const auto& l = p.eye[0].px;
  const auto& r = p.eye[1].px;
  return std::memcmp(l.data(), r.data(), l.size()) != 0;
}

/// Live: copy last Vulkan color RT into eye slot. False if no RT / copy miss.
bool VkCaptureEye(int eye);
bool VkEye_TakePair(VkEyePair* out);

} // namespace cssvr
