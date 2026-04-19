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

  bool operator==(const Vec3 &other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  Vec3 operator-() const { return Vec3(-x, -y, -z); }

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
  template <typename U> Vec3<U> as() const { return {U(x), U(y), U(z)}; }
  Vec3<T> normalized() const {
    T len = length();
    if (len > 0) {
      return *this / len;
    } else {
      return Vec3<T>(0.0);
    }
  }
};

template <typename T> inline Vec3<T> operator*(T s, const Vec3<T> &v) {
  return v * s;
}

namespace std {
template <typename T> struct hash<Vec3<T>> {
  size_t operator()(const Vec3<T> &v) const {
    size_t h1 = std::hash<T>()(v.x);
    size_t h2 = std::hash<T>()(v.y);
    size_t h3 = std::hash<T>()(v.z);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};
} // namespace std