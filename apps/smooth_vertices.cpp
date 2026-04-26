#include <cstdlib>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/stl_io.hpp"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Expected arguments: /path/to/input.stl num_iters "
                 "/path/to/output.stl"
              << std::endl;
    return 1;
  }
  char *input_path = argv[1];
  size_t num_iters = std::strtoull(argv[2], nullptr, 10);
  char *output_path = argv[3];

  auto tris = read_stl(input_path);
  auto mesh = Indexed_Tri_Mesh<double>::from_stl_tris(tris);
  std::vector<std::unordered_set<uint32_t>> vertex_graph;
  vertex_graph.resize(mesh.vertices.size());
  for (const auto &t : mesh.tris) {
    vertex_graph[t[0]].insert({t[1], t[2]});
    vertex_graph[t[1]].insert({t[0], t[2]});
    vertex_graph[t[2]].insert({t[0], t[1]});
  }
  std::vector<Vec3<double>> smoothed_vertices;
  smoothed_vertices.resize(mesh.vertices.size(), Vec3<double>(0, 0, 0));
  auto smooth = [&]() {
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
      auto neighbors = vertex_graph[i];
      Vec3<double> mean(0, 0, 0);
      for (const auto &ni : neighbors) {
        auto nv = mesh.vertices[ni];
        mean += nv;
      }
      mean /= neighbors.size();
      smoothed_vertices[i] = mean;
    }
    mesh.vertices = smoothed_vertices;
  };

  for (int i = 0; i < num_iters; i++) {
    smooth();
  }
  write_stl_binary(output_path, mesh.to_tris());
}