#include <cstdlib>
#include <iostream>

#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/stl_io.hpp"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Expected arguments: /path/to/input.stl fatten_factor "
                 "/path/to/output.stl"
              << std::endl;
    return 1;
  }
  char *input_path = argv[1];
  double fatten_factor = std::strtod(argv[2], nullptr);
  char *output_path = argv[3];

  auto tris = read_stl(input_path);
  auto mesh = Indexed_Tri_Mesh<double>::from_stl_tris(tris);
  auto vertex_normals = mesh.calc_vertex_normals();
  for (size_t i = 0; i < mesh.vertices.size(); i++) {
    auto n = vertex_normals[i];
    mesh.vertices[i] += n * fatten_factor;
  }
  write_stl_binary(output_path, mesh.to_tris());
}