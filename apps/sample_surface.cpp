#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "../libs/stl_io.hpp"

int main(int argc, char *argv[]) {
  if (argc != 5) {
    std::cerr << "Expected arguments: /path/to/input.stl num_samples seed "
                 "/path/to/output.ply"
              << std::endl;
    return 1;
  }

  const char *input_path = argv[1];
  size_t num_samples = std::strtoull(argv[2], nullptr, 10);
  size_t seed = std::strtoull(argv[3], nullptr, 10);
  const char *output_path = argv[4];

  auto t0 = std::chrono::high_resolution_clock::now();
  auto tris = read_stl(input_path);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration<double, std::milli>(t1 - t0);
  std::cout << "Read " << tris.size() << " triangles in " << duration.count()
            << " ms" << std::endl;

  std::vector<double> areas;
  areas.reserve(tris.size());
  for (const auto &t : tris) {
    areas.push_back(t.calc_area());
  }

  std::mt19937_64 rng(seed);
  std::discrete_distribution<size_t> triangle_area_dist(areas.begin(),
                                                        areas.end());
  std::uniform_real_distribution<double> real_dist(0.0, 1.0);

  std::vector<Vec3> samples;
  samples.reserve(num_samples);
  t0 = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < num_samples; i++) {
    size_t triangle_index = triangle_area_dist(rng);
    const Triangle &t = tris[triangle_index];

    // Sample the triangle uniformly:
    // https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#SamplingaTriangle
    double r1 = real_dist(rng);
    double r2 = real_dist(rng);
    double sqrt_r1 = std::sqrt(r1);
    double u = 1 - sqrt_r1;
    double v = r2 * sqrt_r1;

    Vec3 ab = t.b - t.a;
    Vec3 ac = t.c - t.a;

    Vec3 sample_point = u * ab + v * ac + t.a;
    samples.push_back(sample_point);
  }
  t1 = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration<double, std::milli>(t1 - t0);
  std::cout << "Sampled " << num_samples << " points in " << duration.count()
            << " ms" << std::endl;

  std::ofstream out(output_path, std::ios::binary);
  out << "ply\n"
      << "format binary_little_endian 1.0\n"
      << "element vertex " << samples.size() << '\n'
      << "property double x\n"
      << "property double y\n"
      << "property double z\n"
      << "end_header\n";
  out.write(reinterpret_cast<const char *>(samples.data()),
            samples.size() * sizeof(Vec3));
}