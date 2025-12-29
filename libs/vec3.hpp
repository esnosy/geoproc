#pragma once

#include <cmath>
#include <iostream>

struct Vec3 {
  float x, y, z;
  Vec3 operator+(const Vec3 &rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  Vec3 operator-(const Vec3 &rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }
  Vec3 cross(const Vec3 &rhs) const {
    return {y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x};
  }
  Vec3 operator*(const float &rhs) const { return {x * rhs, y * rhs, z * rhs}; }
  float magnitude() const { return std::sqrt(x * x + y * y + z * z); }

  Vec3 min(const Vec3 &rhs) const {
    return {std::min(x, rhs.x), std::min(y, rhs.y), std::min(z, rhs.z)};
  }
  Vec3 max(const Vec3 &rhs) const {
    return {std::max(x, rhs.x), std::max(y, rhs.y), std::max(z, rhs.z)};
  }
  float &operator[](int i) { return (&x)[i]; }
  const float &operator[](int i) const { return (&x)[i]; }
};

inline std::ostream &operator<<(std::ostream &os, const Vec3 &obj) {
  os << "(" << obj.x << ", " << obj.y << ", " << obj.z << ")";
  return os;
}