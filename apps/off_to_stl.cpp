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
  auto indexed_tri_mesh = mesh.triangulate();
  write_stl_binary(output_path, indexed_tri_mesh.to_tris());
}