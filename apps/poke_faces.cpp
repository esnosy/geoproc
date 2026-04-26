#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/stl_io.hpp"
#include "../libs/triangle.hpp"

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
  Indexed_Tri_Mesh<double> subdivided_mesh;
  subdivided_mesh.vertices = mesh.vertices;
  for (const auto &t : mesh.tris) {
    auto v1 = mesh.vertices[t[0]];
    auto v2 = mesh.vertices[t[1]];
    auto v3 = mesh.vertices[t[2]];
    auto center = (v1 + v2 + v3) / 3.0;
    auto center_index = (uint32_t)subdivided_mesh.vertices.size();
    subdivided_mesh.vertices.push_back(center);
    std::array<uint32_t, 3> t1{t[0], t[1], center_index};
    std::array<uint32_t, 3> t2{t[1], t[2], center_index};
    std::array<uint32_t, 3> t3{t[2], t[0], center_index};
    subdivided_mesh.tris.insert(subdivided_mesh.tris.end(), {t1, t2, t3});
  }

  std::vector<Triangle<double>> subdivided_tris;
  subdivided_tris.reserve(subdivided_mesh.tris.size());
  for (const auto &t : subdivided_mesh.tris) {
    auto v1 = subdivided_mesh.vertices[t[0]];
    auto v2 = subdivided_mesh.vertices[t[1]];
    auto v3 = subdivided_mesh.vertices[t[2]];
    subdivided_tris.push_back(Triangle<double>{v1, v2, v3});
  }

  write_stl_binary(output_path, subdivided_tris);
}