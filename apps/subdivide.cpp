#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../libs/indexed_tri_edges_mesh.hpp"
#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/stl_io.hpp"

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
  auto edges_mesh = Indexed_Tri_Edges_Mesh<double>::from_indexed_mesh(mesh);

  std::vector<uint32_t> edge_centers;
  edge_centers.reserve(edges_mesh.edges.size());
  for (const auto &e : edges_mesh.edges) {
    auto v1 = mesh.vertices[e.a];
    auto v2 = mesh.vertices[e.b];
    auto v = (v1 + v2) / 2.0;
    mesh.vertices.push_back(v);
    edge_centers.push_back(mesh.vertices.size() - 1);
  }
  std::vector<std::array<uint32_t, 3>> output_tris;
  output_tris.reserve(tris.size() * 4);

  for (size_t i = 0; i < tris.size(); i++) {
    const auto &te = edges_mesh.tris[i];
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