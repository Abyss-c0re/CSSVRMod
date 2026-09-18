#pragma once
// Source Ray_t / CGameTrace field map for CSS 64-bit. Used only after a live self-test.
#include "collision.hpp"
#include "vec3.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>

namespace cssvr {

constexpr unsigned kMaskSolid = 0x0200400Bu;
constexpr int kTraceWorldOnly = 1;

struct alignas(16) VecAligned {
  float x = 0, y = 0, z = 0, w = 0;
};

struct alignas(16) RayBlob {
  VecAligned start;
  VecAligned delta;
  VecAligned start_off;
  VecAligned extents;
  bool is_ray = true;
  bool is_swept = false;
};

inline void Ray_InitHull(RayBlob* r, const Vec3& start, const Vec3& end, const Vec3& mins,
                         const Vec3& maxs) {
  *r = RayBlob{};
  const Vec3 delta = end - start;
  const Vec3 center = (mins + maxs) * 0.5f;
  Vec3 ext{(maxs.x - mins.x) * 0.5f, (maxs.y - mins.y) * 0.5f, (maxs.z - mins.z) * 0.5f};
  if (ext.x < 0) ext.x = -ext.x;
  if (ext.y < 0) ext.y = -ext.y;
  if (ext.z < 0) ext.z = -ext.z;
  r->delta = {delta.x, delta.y, delta.z, 0.f};
  r->is_swept = delta.LengthSqr() > 1e-8f;
  r->extents = {ext.x, ext.y, ext.z, 0.f};
  r->is_ray = ext.LengthSqr() < 1e-6f;
  r->start = {start.x + center.x, start.y + center.y, start.z + center.z, 0.f};
  r->start_off = {-center.x, -center.y, -center.z, 0.f};
}

// CBaseTrace: startpos@0 endpos@12 plane.normal@24 fraction@44 allsolid@54 startsolid@55
constexpr int kTrStart = 0;
constexpr int kTrEnd = 12;
constexpr int kTrNormal = 24;
constexpr int kTrFraction = 44;
constexpr int kTrAllSolid = 54;
constexpr int kTrStartSolid = 55;

inline bool TraceBlobSane(float fraction, const Vec3& end) {
  if (!std::isfinite(fraction) || fraction < 0.f || fraction > 1.f) return false;
  return std::isfinite(end.x) && std::isfinite(end.y) && std::isfinite(end.z);
}

inline TraceHit TraceHitFromBlob(const unsigned char* blob, size_t n) {
  TraceHit t;
  if (!blob || n < 56) return t;
  std::memcpy(&t.start_pos, blob + kTrStart, 12);
  std::memcpy(&t.end_pos, blob + kTrEnd, 12);
  std::memcpy(&t.hit_normal, blob + kTrNormal, 12);
  std::memcpy(&t.fraction, blob + kTrFraction, 4);
  t.all_solid = blob[kTrAllSolid] != 0;
  t.start_solid = blob[kTrStartSolid] != 0;
  t.hit = t.fraction < 1.f || t.all_solid || t.start_solid;
  t.hit_pos = t.end_pos;
  t.hit_world = t.hit;
  if (!TraceBlobSane(t.fraction, t.end_pos)) {
    t = TraceHit{};
    t.fraction = 1.f;
  }
  return t;
}

} // namespace cssvr
