#pragma once

#include "aabb.hpp"
#include "vec3.hpp"

struct Triangle {
  Vec3 a, b, c;
  Vec3 &operator[](int i) { return (&a)[i]; }
  const Vec3 &operator[](int i) const { return (&a)[i]; }
  double calc_area() const {
    Vec3 ab = b - a;
    Vec3 ac = c - a;
    return 0.5 * ab.cross(ac).length();
  }
  AABB calc_aabb() const { return AABB{a.min(b).min(c), a.max(b).max(c)}; }
};
