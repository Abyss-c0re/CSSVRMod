#pragma once
// Locate ClientModeShared::CreateMove: RTTI + xmm0 (dt) + rsi (CUserCmd*).
#include "view_hook.hpp"
#include <cstdint>
#include <cstring>
#include <vector>

namespace cssvr {

struct CreateMoveLoc {
  bool found = false;
  uint64_t fn_rva = 0;
  int slot = -1;
  std::vector<uint64_t> slot_rva;
  const char* reason = "idle";
};

inline bool LooksLikeCreateMove(const uint8_t* fn, size_t n) {
  if (!fn || n < 24) return false;
  if (!(fn[0] == 0x55 && fn[1] == 0x48 && fn[2] == 0x89 && fn[3] == 0xe5)) return false;
  bool xmm0 = false, rsi = false;
  for (size_t i = 0; i + 5 < n && i < 48; ++i) {
    // movss [rbp+disp8], xmm0
    if (fn[i] == 0xf3 && fn[i + 1] == 0x0f && fn[i + 2] == 0x11 && fn[i + 3] == 0x45) xmm0 = true;
    // mov r12/r13, rsi  (49 89 f4 / 49 89 f5)
    if (fn[i] == 0x49 && fn[i + 1] == 0x89 && (fn[i + 2] == 0xf4 || fn[i + 2] == 0xf5)) rsi = true;
  }
  return xmm0 && rsi;
}

inline bool LocateCreateMoveInImage(const uint8_t* img, size_t len, CreateMoveLoc* out) {
  if (!out) return false;
  *out = CreateMoveLoc{};
  if (!img || len < 64) {
    out->reason = "too_small";
    return false;
  }
  const char* needle = "16ClientModeShared";
  const uint8_t* found = nullptr;
  for (size_t i = 0; i + 18 < len; ++i) {
    if (img[i] == '1' && std::memcmp(img + i, needle, 18) == 0) {
      found = img + i;
      break;
    }
  }
  if (!found) {
    out->reason = "no_rtti";
    return false;
  }
  const auto map = detail::ParseElf(img, len);
  const uint64_t str_va = map.OffToVa((uint64_t)(found - img));
  if (str_va == ~uint64_t{0}) {
    out->reason = "no_str_va";
    return false;
  }
  // typeinfo.name at +8 → typeinfo at name-8. vtable[-1] points at typeinfo.
  std::vector<uint64_t> name_slots;
  for (const auto& sec : map.load) {
    const size_t begin = (size_t)sec.off;
    const size_t last = begin + (size_t)sec.sz;
    const size_t end = last < len ? last : len;
    for (size_t i = begin; i + 8 <= end; i += 8) {
      if (detail::RvU64(img + i) == str_va) name_slots.push_back(map.OffToVa(i));
    }
  }
  if (name_slots.empty()) {
    out->reason = "no_typeinfo";
    return false;
  }
  std::vector<uint64_t> vt_minus1;
  for (uint64_t name_slot : name_slots) {
    const uint64_t typeinfo = name_slot - 8;
    for (const auto& sec : map.load) {
      if (map.is_elf && !sec.wr) continue;
      const size_t begin = (size_t)sec.off;
      const size_t last = begin + (size_t)sec.sz;
      const size_t end = last < len ? last : len;
      for (size_t i = begin; i + 8 <= end; i += 8) {
        if (detail::RvU64(img + i) == typeinfo) vt_minus1.push_back(map.OffToVa(i));
      }
    }
  }
  if (vt_minus1.empty()) {
    out->reason = "no_vtable";
    return false;
  }
  for (uint64_t vm1 : vt_minus1) {
    const uint64_t vt = vm1 + 8;
    for (int slot = 16; slot <= 26; ++slot) {
      const uint64_t slot_va = vt + (uint64_t)slot * 8;
      const uint64_t off = map.VaToOff(slot_va);
      if (off == ~uint64_t{0} || off + 8 > len) continue;
      const uint64_t fn = detail::RvU64(img + off);
      const uint64_t fn_off = map.VaToOff(fn);
      if (fn_off == ~uint64_t{0} || fn_off + 32 > len) continue;
      if (!LooksLikeCreateMove(img + fn_off, 48)) continue;
      if (out->found && out->fn_rva != fn) continue;
      out->found = true;
      out->fn_rva = fn;
      out->slot = slot;
      out->slot_rva.push_back(slot_va);
      out->reason = "located";
    }
  }
  if (!out->found) {
    out->reason = "no_xmm0_rsi";
    return false;
  }
  return true;
}

inline bool LocateCreateMoveFile(const char* path, CreateMoveLoc* out) {
  if (!path || !out) return false;
  *out = CreateMoveLoc{};
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
  return LocateCreateMoveInImage(buf.data(), buf.size(), out);
}

// Honest toast if CreateMove locate/hook misses — analog overlay never applies.
struct CreateMoveToastIn {
  const char* locate_reason = "idle";
  bool hooked = false;
  bool already_shown = false;
};

struct CreateMoveToast {
  bool should_toast = false;
  bool abort_vr = false;
  const char* reason = "idle";
  const char* copy = "";
  const char* label = "CM · IDLE";
};

inline bool CreateMove_IsMissReason(const char* reason) {
  if (!reason || !reason[0]) return false;
  if (std::strcmp(reason, "idle") == 0 || std::strcmp(reason, "located") == 0 ||
      std::strcmp(reason, "hooked") == 0)
    return false;
  return true;
}

inline const char* CreateMove_MissCopy(const char* reason) {
  if (reason && (std::strcmp(reason, "no_css") == 0 || std::strcmp(reason, "open_fail") == 0 ||
                 std::strcmp(reason, "read_fail") == 0 || std::strcmp(reason, "bad_size") == 0))
    return "CSS client.so missing — cannot hook CreateMove (stick stays keyboard).";
  if (reason && (std::strcmp(reason, "no_rtti") == 0 || std::strcmp(reason, "no_typeinfo") == 0 ||
                 std::strcmp(reason, "no_vtable") == 0 || std::strcmp(reason, "no_xmm0_rsi") == 0))
    return "CreateMove not found — analog overlay never applies.";
  if (reason && (std::strcmp(reason, "no_patch") == 0 || std::strcmp(reason, "no_client_base") == 0))
    return "CreateMove hook missed — analog overlay never applies.";
  return "CreateMove locate failed — analog overlay never applies.";
}

inline CreateMoveToast CreateMove_ToastDecide(const CreateMoveToastIn& in) {
  CreateMoveToast t;
  t.abort_vr = false;
  t.reason = (in.locate_reason && in.locate_reason[0]) ? in.locate_reason : "idle";
  if (in.hooked) {
    t.reason = "hooked";
    t.label = "CM · HOOKED";
    return t;
  }
  t.copy = CreateMove_MissCopy(t.reason);
  t.label = CreateMove_IsMissReason(t.reason) ? "CM · MISS" : "CM · IDLE";
  t.should_toast = !in.already_shown && CreateMove_IsMissReason(t.reason);
  return t;
}

} // namespace cssvr
