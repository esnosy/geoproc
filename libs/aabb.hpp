#pragma once

#include "vec3.hpp"

struct AABB {
  Vec3 min;
  Vec3 max;

  AABB() : min(Vec3(0.0)), max(Vec3(0.0)) {}
  AABB(const Vec3 &min, const Vec3 &max) : min(min), max(max) {}
};