#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "../libs/aabb.hpp"
#include "../libs/stl_io.hpp"

// struct BVH

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/input.stl" << std::endl;
    return 1;
  }

  const char *input_path = argv[1];

  auto t0 = std::chrono::high_resolution_clock::now();
  auto tris = read_stl(input_path);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration<double, std::milli>(t1 - t0);
  std::cout << "Read " << tris.size() << " triangles in " << duration.count()
            << " ms" << std::endl;

  std::vector<AABB> aabbs;
  aabbs.reserve(tris.size());
  for (const auto &tri : tris) {
    aabbs.push_back(tri.calc_aabb());
  }
}