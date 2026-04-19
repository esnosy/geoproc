#pragma once

#include "aabb.hpp"
#include "vec3.hpp"

template <typename T> struct Triangle {
  Vec3<T> a, b, c;
  Vec3<T> &operator[](int i) { return (&a)[i]; }
  const Vec3<T> &operator[](int i) const { return (&a)[i]; }
  T calc_area() const {
    Vec3<T> ab = b - a;
    Vec3<T> ac = c - a;
    return 0.5 * ab.cross(ac).length();
  }
  AABB<T> calc_aabb() const {
    return AABB<T>{a.min(b).min(c), a.max(b).max(c)};
  }
  Vec3<T> calc_normal() const {
    Vec3<T> ab = b - a;
    Vec3<T> ac = c - a;
    return ab.cross(ac).normalized();
  }
  Vec3<T> calc_normal_unnormalized() const {
    Vec3<T> ab = b - a;
    Vec3<T> ac = c - a;
    return ab.cross(ac);
  }
};
