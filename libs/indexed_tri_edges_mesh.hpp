#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "indexed_tri_mesh.hpp"

union Undirected_Edge {
  uint64_t combined;
  struct {
    uint32_t a, b;
  };
  Undirected_Edge(uint32_t a_, uint32_t b_) {
    if (a_ > b_) {
      a = b_;
      b = a_;
    } else {
      a = a_;
      b = b_;
    }
  }
  bool operator==(const Undirected_Edge &other) const {
    return combined == other.combined;
  }
};
namespace std {
template <> struct hash<Undirected_Edge> {
  size_t operator()(const Undirected_Edge &e) const { return e.combined; }
};
} // namespace std

template <typename T> struct Indexed_Tri_Edges_Mesh {
  std::vector<Vec3<T>> vertices;
  std::vector<Undirected_Edge> edges;
  std::vector<std::array<uint32_t, 3>> tris;

  static Indexed_Tri_Edges_Mesh<T>
  from_indexed_mesh(const Indexed_Tri_Mesh<T> &mesh) {
    std::vector<Undirected_Edge> edges;
    edges.reserve(mesh.tris.size() * 3);
    std::unordered_map<Undirected_Edge, uint32_t> edge_to_index;
    edge_to_index.reserve(mesh.tris.size() * 3);

    std::vector<std::array<uint32_t, 3>> triangles_of_edges;
    triangles_of_edges.reserve(mesh.tris.size());

    for (const auto &t : mesh.tris) {
      std::array<uint32_t, 3> triangle_of_edges;
      for (int i = 0; i < 3; i++) {
        auto e = Undirected_Edge(t[i], t[(i + 1) % 3]);
        auto it = edge_to_index.find(e);
        if (it == edge_to_index.end()) {
          auto unique_edge_index = edges.size();
          edge_to_index[e] = unique_edge_index;
          triangle_of_edges[i] = unique_edge_index;
          edges.push_back(e);
        } else {
          triangle_of_edges[i] = it->second;
        }
      }
      triangles_of_edges.push_back(triangle_of_edges);
    }

    return Indexed_Tri_Edges_Mesh<T>{mesh.vertices, edges, triangles_of_edges};
  }
};