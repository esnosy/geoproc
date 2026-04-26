#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "triangle.hpp"
#include "vec3.hpp"

template <typename T> struct Indexed_Tri_Mesh {
  std::vector<Vec3<T>> vertices;
  std::vector<std::array<uint32_t, 3>> tris;

  static Indexed_Tri_Mesh<T>
  from_stl_tris(const std::vector<Triangle<T>> &tris) {
    std::unordered_map<Vec3<T>, uint32_t> vertex_to_index;
    vertex_to_index.reserve(tris.size() * 3);
    Indexed_Tri_Mesh<T> mesh;
    mesh.tris.reserve(tris.size());
    mesh.vertices.reserve(tris.size() * 3);
    for (const auto &t : tris) {
      std::array<uint32_t, 3> indexed_tri;

      for (int i = 0; i < 3; i++) {
        auto v_it = vertex_to_index.find(t[i]);
        if (v_it == vertex_to_index.end()) {
          auto unique_vertex_index = mesh.vertices.size();
          vertex_to_index[t[i]] = unique_vertex_index;
          indexed_tri[i] = unique_vertex_index;
          mesh.vertices.push_back(t[i]);
        } else {
          indexed_tri[i] = v_it->second;
        }
      }
      mesh.tris.push_back(indexed_tri);
    }
    return mesh;
  }

  std::vector<Triangle<T>> to_tris() const {
    std::vector<Triangle<T>> tris_out;
    tris_out.reserve(tris.size());
    for (const auto &t : tris) {
      auto v1 = vertices[t[0]];
      auto v2 = vertices[t[1]];
      auto v3 = vertices[t[2]];
      tris_out.push_back(Triangle<T>{v1, v2, v3});
    }
    return tris_out;
  }

  std::vector<Vec3<T>> calc_vertex_normals() const {
    std::vector<Vec3<T>> vertex_normals;
    vertex_normals.resize(vertices.size(), Vec3<T>(0, 0, 0));
    for (size_t i = 0; i < tris.size(); i++) {
      const auto &indexed_tri = tris[i];
      const auto &tri =
          Triangle<T>{vertices[indexed_tri[0]], vertices[indexed_tri[1]],
                      vertices[indexed_tri[2]]};
      const auto &n = tri.calc_normal_unnormalized();
      for (int j = 0; j < 3; j++) {
        vertex_normals[indexed_tri[j]] += n;
      }
    }
    for (auto &n : vertex_normals) {
      n = n.normalized();
    }
    return vertex_normals;
  }
};