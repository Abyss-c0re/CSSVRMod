#pragma once
// CSS 64-bit CViewSetup fields. Offsets from CViewRender::RenderView copies
// (r13 = setup): ints at 0/8/0x10/0x18, fov 0x38, origin 0x40, angles 0x4c.
#include "vec3.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace cssvr {

struct ViewSetupFields {
  int origin_off = 0x40;
  int angles_off = 0x4c;
  int fov_off = 0x38;
  int width_off = 0x10;
  int height_off = 0x18;
};

inline bool ViewSetup_ReadVec(const void* blob, size_t n, int off, Vec3* o) {
  if (!blob || !o || off < 0 || (size_t)off + 12 > n) return false;
  std::memcpy(o, static_cast<const char*>(blob) + off, 12);
  return true;
}

inline bool ViewSetup_WriteVec(void* blob, size_t n, int off, const Vec3& v) {
  if (!blob || off < 0 || (size_t)off + 12 > n) return false;
  std::memcpy(static_cast<char*>(blob) + off, &v, 12);
  return true;
}

inline bool ViewSetup_ReadFloat(const void* blob, size_t n, int off, float* f) {
  if (!blob || !f || off < 0 || (size_t)off + 4 > n) return false;
  std::memcpy(f, static_cast<const char*>(blob) + off, 4);
  return true;
}

inline bool ViewSetup_ReadInt(const void* blob, size_t n, int off, int* v) {
  if (!blob || !v || off < 0 || (size_t)off + 4 > n) return false;
  std::memcpy(v, static_cast<const char*>(blob) + off, 4);
  return true;
}

inline bool ViewSetup_ReadPose(const void* blob, size_t n, const ViewSetupFields& f, Vec3* origin,
                               Ang3* angles, float* fov) {
  Vec3 o, a;
  float fv = 90.f;
  if (!ViewSetup_ReadVec(blob, n, f.origin_off, &o)) return false;
  if (!ViewSetup_ReadVec(blob, n, f.angles_off, &a)) return false;
  if (fov && !ViewSetup_ReadFloat(blob, n, f.fov_off, &fv)) return false;
  if (origin) *origin = o;
  if (angles) {
    angles->p = a.x;
    angles->y = a.y;
    angles->r = a.z;
  }
  if (fov) *fov = fv;
  return true;
}

inline bool ViewSetup_WriteOrigin(void* blob, size_t n, const ViewSetupFields& f, const Vec3& o) {
  return ViewSetup_WriteVec(blob, n, f.origin_off, o);
}

} // namespace cssvr
