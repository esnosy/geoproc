#include <chrono>
#include <iostream>

#include "../libs/stl_io.hpp"

int main(int argc, char *argv[]) {
  char *filename = argv[1];
  auto t0 = std::chrono::high_resolution_clock::now();
  auto tris = read_stl(filename);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto dur = t1 - t0;
  auto dur_ms_count =
      std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
  std::cout << "Read " << tris.size() << " tris in " << dur_ms_count << "ms"
            << std::endl;
}