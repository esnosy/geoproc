#pragma once

#include <cmath>

// Account for Windows headers
#undef min
#undef max

template <typename T> struct Vec3 {
  T x, y, z;
  Vec3(float buf[3]) : x(buf[0]), y(buf[1]), z(buf[2]) {}
  Vec3() : x(0), y(0), z(0) {}
  Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
  Vec3(T v) : x(v), y(v), z(v) {}
  T &operator[](int i) { return (&x)[i]; }
  T operator[](int i) const { return (&x)[i]; }

  void operator+=(const Vec3 &other) {
    x += other.x;
    y += other.y;
    z += other.z;
  }
  void operator/=(T s) {
    x /= s;
    y /= s;
    z /= s;
  }
  Vec3 operator*(const Vec3 &other) const {
    return Vec3{x * other.x, y * other.y, z * other.z};
  }
  Vec3 operator*(T s) const { return Vec3{x * s, y * s, z * s}; }
  Vec3 operator/(T s) const { return Vec3{x / s, y / s, z / s}; }
  Vec3 operator+(const Vec3 &other) const {
    return Vec3{x + other.x, y + other.y, z + other.z};
  }
  Vec3 operator-(const Vec3 &other) const {
    return Vec3{x - other.x, y - other.y, z - other.z};
  }

  T dot(const Vec3 &other) const {
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

  T length_squared() const { return this->dot(*this); }
  T length() const { return std::sqrt(length_squared()); }
  Vec3<float> as_float() const { return {float(x), float(y), float(z)}; }
  Vec3<double> as_double() const { return {double(x), double(y), double(z)}; }
};

template <typename T> inline Vec3<T> operator*(T s, const Vec3<T> &v) {
  return v * s;
}