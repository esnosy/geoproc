#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/stl_io.hpp"

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

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Expected arguments: /path/to/input.stl /path/to/output.stl"
              << std::endl;
    return 1;
  }
  char *input_path = argv[1];
  char *output_path = argv[2];

  auto tris = read_stl(input_path);
  auto mesh = Indexed_Tri_Mesh<double>::from_stl_tris(tris);

  std::vector<Undirected_Edge> edges;
  edges.reserve(mesh.tris.size() * 3);
  std::unordered_map<Undirected_Edge, uint32_t> edge_to_index;
  edge_to_index.reserve(mesh.tris.size() * 3);

  std::vector<std::array<uint32_t, 3>> triangles_of_edges;
  triangles_of_edges.reserve(tris.size());

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

  std::vector<uint32_t> edge_centers;
  edge_centers.reserve(edges.size());
  for (const auto &e : edges) {
    auto v1 = mesh.vertices[e.a];
    auto v2 = mesh.vertices[e.b];
    auto v = (v1 + v2) / 2.0;
    mesh.vertices.push_back(v);
    edge_centers.push_back(mesh.vertices.size() - 1);
  }
  std::vector<std::array<uint32_t, 3>> output_tris;
  output_tris.reserve(tris.size() * 4);

  for (size_t i = 0; i < tris.size(); i++) {
    const auto &te = triangles_of_edges[i];
    auto ec1 = edge_centers[te[0]];
    auto ec2 = edge_centers[te[1]];
    auto ec3 = edge_centers[te[2]];
    const auto &t = mesh.tris[i];
    output_tris.push_back({ec1, ec2, ec3});
    output_tris.push_back({t[0], ec1, ec3});
    output_tris.push_back({t[1], ec2, ec1});
    output_tris.push_back({t[2], ec3, ec2});
  }
  mesh.tris = output_tris;
  write_stl_binary(output_path, mesh.to_tris());
}