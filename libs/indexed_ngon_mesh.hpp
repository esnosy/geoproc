#pragma once

#include <cstdint>
#include <vector>

#include "indexed_tri_mesh.hpp"
#include "vec3.hpp"

template <typename T> struct Indexed_Ngon_Mesh {
  std::vector<Vec3<T>> vertices;
  std::vector<std::vector<uint32_t>> faces;

  Indexed_Tri_Mesh<T> triangulate() {
    Indexed_Tri_Mesh<double> result;
    result.vertices = vertices;
    for (const auto &f : faces) {
      if (f.size() == 3) {
        std::array<uint32_t, 3> t;
        t[0] = f[0];
        t[1] = f[1];
        t[2] = f[2];
        result.tris.push_back(t);
      } else if (f.size() > 3) {
        for (size_t i = 2; i < f.size(); i++) {
          std::array<uint32_t, 3> t;
          t[0] = f[0];
          t[1] = f[i - 1];
          t[2] = f[i];
          result.tris.push_back(t);
        }
      } else {
        // do nothing
      }
    }
    return result;
  }
};