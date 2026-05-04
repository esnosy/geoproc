#pragma once

#include <cstdint>
#include <vector>

#include "vec3.hpp"

template <typename T> struct Indexed_Ngon_Mesh {
  std::vector<Vec3<T>> vertices;
  std::vector<std::vector<uint32_t>> faces;
};