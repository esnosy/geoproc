#pragma once

#include "vec3.hpp"

struct AABB {
  Vec3 min;
  Vec3 max;

  AABB() : min(Vec3(0.0)), max(Vec3(0.0)) {}
  AABB(const Vec3 &min, const Vec3 &max) : min(min), max(max) {}
  AABB join(const AABB &other) const {
    return AABB{min.min(other.min), max.max(other.max)};
  }
  Vec3 calc_extent() const { return max - min; }
  double calc_axis_center(int axis) const {
    return (min[axis] + max[axis]) / 2;
  }
  Vec3 calc_center() const { return (min + max) / 2; }
};