#pragma once
// Locate CSS CViewRender::RenderView by string xref — no guessed vtable index.
#include "view_setup.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace cssvr {

struct RenderViewLoc {
  bool found = false;
  uint64_t fn_rva = 0;
  uint64_t str_rva = 0;
  ViewSetupFields fields;
  std::vector<uint64_t> slot_rva; // vtable slots that point at fn
  const char* reason = "idle";
};

namespace detail {

inline uint16_t RvU16(const uint8_t* p) {
  uint16_t v;
  std::memcpy(&v, p, 2);
  return v;
}
inline uint32_t RvU32(const uint8_t* p) {
  uint32_t v;
  std::memcpy(&v, p, 4);
  return v;
}
inline uint64_t RvU64(const uint8_t* p) {
  uint64_t v;
  std::memcpy(&v, p, 8);
  return v;
}

struct ElfMap {
  bool ok = false;
  const uint8_t* img = nullptr;
  size_t len = 0;
  bool is_elf = false;
  struct Sec {
    uint64_t addr = 0, off = 0, sz = 0;
    bool wr = true;
  };
  std::vector<Sec> load;
  uint64_t VaToOff(uint64_t va) const {
    for (const auto& s : load) {
      if (va >= s.addr && va < s.addr + s.sz) return s.off + (va - s.addr);
    }
    if (!is_elf && va < len) return va;
    return ~uint64_t{0};
  }
  uint64_t OffToVa(uint64_t off) const {
    for (const auto& s : load) {
      if (off >= s.off && off < s.off + s.sz) return s.addr + (off - s.off);
    }
    if (!is_elf && off < len) return off;
    return ~uint64_t{0};
  }
};

inline ElfMap ParseElf(const uint8_t* img, size_t len) {
  ElfMap m;
  m.img = img;
  m.len = len;
  if (len >= 64 && img[0] == 0x7f && img[1] == 'E' && img[2] == 'L' && img[3] == 'F' &&
      img[4] == 2) {
    m.is_elf = true;
    const uint64_t phoff = RvU64(img + 32);
    const uint16_t phentsize = RvU16(img + 54);
    const uint16_t phnum = RvU16(img + 56);
    for (uint16_t i = 0; i < phnum; ++i) {
      const size_t o = (size_t)phoff + (size_t)i * phentsize;
      if (o + 56 > len) break;
      if (RvU32(img + o) != 1) continue; // PT_LOAD
      ElfMap::Sec s;
      s.off = RvU64(img + o + 8);
      s.addr = RvU64(img + o + 16);
      s.sz = RvU64(img + o + 32);
      s.wr = (RvU32(img + o + 4) & 2) != 0;
      m.load.push_back(s);
    }
    m.ok = !m.load.empty();
  } else {
    ElfMap::Sec s;
    s.off = 0;
    s.addr = 0;
    s.sz = len;
    m.load.push_back(s);
    m.ok = len > 32;
  }
  return m;
}

} // namespace detail

/// Scan a CSS client.so image (file or a flat fixture). Self-test: string + prologue + origin load.
inline bool LocateRenderViewInImage(const uint8_t* img, size_t len, RenderViewLoc* out) {
  if (!out) return false;
  *out = RenderViewLoc{};
  if (!img || len < 64) {
    out->reason = "too_small";
    return false;
  }
  const char* needle = "CViewRender::RenderView";
  const size_t nlen = std::strlen(needle);
  const uint8_t* found = nullptr;
  for (size_t i = 0; i + nlen < len; ++i) {
    if (img[i] == 'C' && std::memcmp(img + i, needle, nlen) == 0) {
      found = img + i;
      break;
    }
  }
  if (!found) {
    out->reason = "no_string";
    return false;
  }
  const auto map = detail::ParseElf(img, len);
  const uint64_t str_off = (uint64_t)(found - img);
  const uint64_t str_va = map.OffToVa(str_off);
  if (str_va == ~uint64_t{0}) {
    out->reason = "no_str_va";
    return false;
  }
  out->str_rva = str_va;

  uint64_t xref_va = 0;
  for (size_t i = 0; i + 7 < len; ++i) {
    if (!(img[i] == 0x48 && img[i + 1] == 0x8d && (img[i + 2] & 0xC7) == 0x05)) continue;
    int32_t disp = 0;
    std::memcpy(&disp, img + i + 3, 4);
    const uint64_t insn_va = map.OffToVa(i);
    if (insn_va == ~uint64_t{0}) continue;
    const uint64_t tgt = insn_va + 7 + (int64_t)disp;
    if (tgt == str_va) {
      xref_va = insn_va;
      break;
    }
  }
  if (!xref_va) {
    out->reason = "no_xref";
    return false;
  }

  uint64_t fn_va = 0;
  const uint64_t xref_off = map.VaToOff(xref_va);
  if (xref_off == ~uint64_t{0} || xref_off > len) {
    out->reason = "bad_xref";
    return false;
  }
  const size_t back = xref_off > 0x3000 ? xref_off - 0x3000 : 0;
  for (size_t o = xref_off; o-- > back;) {
    if (img[o] == 0x55 && o + 5 < len && img[o + 1] == 0x48 && img[o + 2] == 0x89 &&
        img[o + 3] == 0xe5) {
      fn_va = map.OffToVa(o);
      break;
    }
  }
  if (!fn_va) {
    out->reason = "no_prologue";
    return false;
  }
  const uint64_t fn_off = map.VaToOff(fn_va);
  if (fn_off == ~uint64_t{0} || fn_off + 0x180 > len) {
    out->reason = "bad_fn";
    return false;
  }
  bool saw_origin = false;
  bool saw_rsi = false;
  for (size_t o = fn_off; o + 6 < fn_off + 0x180 && o + 6 < len; ++o) {
    if (img[o] == 0x49 && img[o + 1] == 0x89 && img[o + 2] == 0xf5) saw_rsi = true;
    // movss xmm0, dword [r13+0x40]
    if (img[o] == 0xf3 && img[o + 1] == 0x41 && img[o + 2] == 0x0f && img[o + 3] == 0x10 &&
        img[o + 4] == 0x45 && img[o + 5] == 0x40)
      saw_origin = true;
  }
  if (!saw_origin) {
    out->reason = "no_origin_load";
    return false;
  }
  (void)saw_rsi;
  out->fn_rva = fn_va;
  out->fields.origin_off = 0x40;
  out->fields.angles_off = 0x4c;
  out->fields.fov_off = 0x38;

  for (const auto& sec : map.load) {
    if (map.is_elf && !sec.wr) continue;
    const size_t begin = (size_t)sec.off;
    const size_t end = begin + (size_t)sec.sz;
    if (begin >= len) continue;
    const size_t last = end < len ? end : len;
    for (size_t i = begin; i + 8 <= last; i += 8) {
      if (detail::RvU64(img + i) == fn_va) {
        const uint64_t va = map.OffToVa(i);
        if (va != ~uint64_t{0}) out->slot_rva.push_back(va);
      }
    }
  }
  out->found = true;
  out->reason = out->slot_rva.empty() ? "fn_no_vtable" : "located";
  return true;
}

void ViewHookTryInstall();
void ViewHookOnSwap();
bool ViewHookTakeEyes(unsigned* l, unsigned* r, int* w, int* h);
bool ViewHookHaveEyes();

// Honest toast if locate/hook misses — dual paint never starts (stay MONO).
struct RenderViewToastIn {
  const char* locate_reason = "idle";
  bool hooked = false;
  bool already_shown = false;
};

struct RenderViewToast {
  bool should_toast = false;
  bool abort_vr = false;
  const char* reason = "idle";
  const char* copy = "";
  const char* label = "RV · IDLE";
};

inline bool RenderView_IsMissReason(const char* reason) {
  if (!reason || !reason[0]) return false;
  if (std::strcmp(reason, "idle") == 0 || std::strcmp(reason, "located") == 0 ||
      std::strcmp(reason, "hooked") == 0)
    return false;
  return true;
}

inline const char* RenderView_MissCopy(const char* reason) {
  if (reason && (std::strcmp(reason, "no_css") == 0 || std::strcmp(reason, "open_fail") == 0 ||
                 std::strcmp(reason, "read_fail") == 0 || std::strcmp(reason, "bad_size") == 0))
    return "CSS client.so missing — cannot hook CViewRender (staying MONO).";
  if (reason && (std::strcmp(reason, "no_string") == 0 || std::strcmp(reason, "no_xref") == 0 ||
                 std::strcmp(reason, "no_prologue") == 0 || std::strcmp(reason, "no_origin_load") == 0))
    return "CViewRender::RenderView not found — dual paint never starts (MONO).";
  if (reason && (std::strcmp(reason, "fn_no_vtable") == 0 || std::strcmp(reason, "no_patch") == 0 ||
                 std::strcmp(reason, "no_client_base") == 0))
    return "RenderView hook missed — dual paint never starts (MONO).";
  return "CViewRender locate failed — dual paint never starts (MONO).";
}

inline RenderViewToast RenderView_ToastDecide(const RenderViewToastIn& in) {
  RenderViewToast t;
  t.abort_vr = false;
  t.reason = (in.locate_reason && in.locate_reason[0]) ? in.locate_reason : "idle";
  if (in.hooked) {
    t.reason = "hooked";
    t.label = "RV · HOOKED";
    return t;
  }
  t.copy = RenderView_MissCopy(t.reason);
  t.label = RenderView_IsMissReason(t.reason) ? "RV · MISS" : "RV · IDLE";
  t.should_toast = !in.already_shown && RenderView_IsMissReason(t.reason);
  return t;
}

inline bool LocateRenderViewFile(const char* path, RenderViewLoc* out) {
  if (!path || !out) return false;
  *out = RenderViewLoc{};
  FILE* f = std::fopen(path, "rb");
  if (!f) {
    out->reason = "open_fail";
    return false;
  }
  std::fseek(f, 0, SEEK_END);
  const long sz = std::ftell(f);
  std::fseek(f, 0, SEEK_SET);
  if (sz < 64 || sz > 80 * 1024 * 1024) {
    std::fclose(f);
    out->reason = "bad_size";
    return false;
  }
  std::vector<uint8_t> buf((size_t)sz);
  if (std::fread(buf.data(), 1, buf.size(), f) != buf.size()) {
    std::fclose(f);
    out->reason = "read_fail";
    return false;
  }
  std::fclose(f);
  return LocateRenderViewInImage(buf.data(), buf.size(), out);
}

} // namespace cssvr
