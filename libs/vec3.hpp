#pragma once

#include <cmath>

struct Vec3 {
  double x, y, z;
  Vec3(float buf[3]) : x(buf[0]), y(buf[1]), z(buf[2]) {}
  Vec3() : x(0), y(0), z(0) {}
  Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
  Vec3(double v) : x(v), y(v), z(v) {}
  double &operator[](int i) { return (&x)[i]; }

  Vec3 operator*(double s) const { return Vec3{x * s, y * s, z * s}; }
  Vec3 operator/(double s) const { return Vec3{x / s, y / s, z / s}; }
  Vec3 operator+(const Vec3 &other) const {
    return Vec3{x + other.x, y + other.y, z + other.z};
  }
  Vec3 operator-(const Vec3 &other) const {
    return Vec3{x - other.x, y - other.y, z - other.z};
  }

  double dot(const Vec3 &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  Vec3 cross(const Vec3 &other) const {
    return Vec3{
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x,
    };
  }
  Vec3 min(const Vec3 &other) const {
    return Vec3{
        std::min(x, other.x),
        std::min(y, other.y),
        std::min(z, other.z),
    };
  }
  Vec3 max(const Vec3 &other) const {
    return Vec3{
        std::max(x, other.x),
        std::max(y, other.y),
        std::max(z, other.z),
    };
  }

  double length_squared() const { return this->dot(*this); }
  double length() const { return std::sqrt(length_squared()); }
};

inline Vec3 operator*(double s, const Vec3 &v) { return v * s; }