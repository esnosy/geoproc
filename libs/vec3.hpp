#pragma once

#include <cmath>

struct vec3
{
  float x, y, z;
  vec3 operator+(const vec3 &rhs) const
  {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  vec3 operator-(const vec3 &rhs) const
  {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }
  vec3 cross(const vec3 &rhs) const
  {
    return {y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x};
  }
  vec3 operator*(const float &rhs) const { return {x * rhs, y * rhs, z * rhs}; }
  float magnitude() const { return sqrt(x * x + y * y + z * z); }
};