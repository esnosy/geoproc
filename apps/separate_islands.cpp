#include <cstdint>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/stl_io.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/input.stl" << std::endl;
    return 1;
  }
  auto tris = read_stl(argv[1]);
  auto mesh = Indexed_Tri_Mesh<double>::from_stl_tris(tris);
  std::vector<bool> tris_visited(mesh.tris.size(), false);
  std::vector<bool> vertices_visited(mesh.vertices.size(), false);
  std::vector<std::vector<uint32_t>> per_vertex_neighbour_tris;
  per_vertex_neighbour_tris.resize(mesh.vertices.size());
  for (size_t i = 0; i < mesh.tris.size(); i++) {
    const auto &t = mesh.tris[i];
    for (int j = 0; j < 3; j++) {
      per_vertex_neighbour_tris[t[j]].push_back(i);
    }
  }
  std::vector<Indexed_Tri_Mesh<double>> islands;
  for (size_t i = 0; i < mesh.vertices.size(); i++) {
    if (vertices_visited[i]) {
      continue;
    }
    Indexed_Tri_Mesh<double> island;
    island.vertices = mesh.vertices;
    std::stack<size_t> stack;
    stack.push(i);
    while (!stack.empty()) {
      auto vi = stack.top();
      stack.pop();
      if (vertices_visited[vi]) {
        continue;
      }
      vertices_visited[vi] = true;
      for (const auto &ti : per_vertex_neighbour_tris[vi]) {
        if (tris_visited[ti]) {
          continue;
        }
        const auto &t = mesh.tris[ti];
        island.tris.push_back(t);
        tris_visited[ti] = true;
        stack.push(t[0]);
        stack.push(t[1]);
        stack.push(t[2]);
      }
    }
    islands.push_back(island);
  }

  for (size_t i = 0; i < islands.size(); i++) {
    std::string filename = "island_" + std::to_string(i) + ".stl";
    write_stl_binary(filename.c_str(), islands[i].to_tris());
  }
}