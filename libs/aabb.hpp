#pragma once

#include "Vec3.hpp"

template <typename T> struct AABB {
  Vec3<T> min;
  Vec3<T> max;

  AABB() : min(Vec3<T>(0.0)), max(Vec3<T>(0.0)) {}
  AABB(const Vec3<T> &min, const Vec3<T> &max) : min(min), max(max) {}
  AABB join(const AABB &other) const {
    return AABB{min.min(other.min), max.max(other.max)};
  }
  Vec3<T> calc_extent() const { return max - min; }
  double calc_axis_center(int axis) const {
    return (min[axis] + max[axis]) / 2;
  }
  Vec3<T> calc_center() const { return (min + max) / 2; }
};