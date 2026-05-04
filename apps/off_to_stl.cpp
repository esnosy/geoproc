#include <array>
#include <cstdint>
#include <iostream>

#include "../libs/indexed_ngon_mesh.hpp"
#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/off_io.hpp"
#include "../libs/stl_io.hpp"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Expected arguments: /path/to/input.off /path/to/output.stl"
              << std::endl;
    return 1;
  }

  char *input_path = argv[1];
  char *output_path = argv[2];

  auto mesh = read_off<double>(input_path);
  Indexed_Tri_Mesh<double> indexed_tri_mesh;
  indexed_tri_mesh.vertices = mesh.vertices;
  for (const auto &f : mesh.faces) {
    if (f.size() == 3) {
      std::array<uint32_t, 3> t;
      t[0] = f[0];
      t[1] = f[1];
      t[2] = f[2];
      indexed_tri_mesh.tris.push_back(t);
    } else if (f.size() > 3) {
      for (size_t i = 2; i < f.size(); i++) {
        std::array<uint32_t, 3> t;
        t[0] = f[0];
        t[1] = f[i - 1];
        t[2] = f[i];
        indexed_tri_mesh.tris.push_back(t);
      }
    } else {
      // do nothing
    }
  }

  write_stl_binary(output_path, indexed_tri_mesh.to_tris());
}