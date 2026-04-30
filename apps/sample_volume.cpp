#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include "../libs/aabb.hpp"
#include "../libs/bvh.hpp"
#include "../libs/ply_io.hpp"
#include "../libs/stl_io.hpp"
#include "../libs/vec3.hpp"

static const float pi = 3.14159265358979323846;

Vec3<float> uniform_sample_sphere(std::array<float, 2> u) {
  float z = 1.0f - 2.0f * u[0];
  float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
  float phi = 2.0f * pi * u[1];
  return Vec3<float>(r * std::cos(phi), r * std::sin(phi), z);
}

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

  // Generate random directions

  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  auto num_dirs = 32;
  std::vector<Vec3<float>> directions;
  directions.reserve(num_dirs);
  for (int i = 0; i < num_dirs; i++) {
    std::array<float, 2> u;
    u[0] = dist(rng);
    u[1] = dist(rng);
    auto dir = uniform_sample_sphere(u);
    directions.push_back(dir);
  }

  auto tris = read_stl(input_path);

  std::vector<Triangle<float>> tris_float;
  tris_float.reserve(tris.size());
  for (const auto &t : tris) {
    tris_float.push_back({t.a.as<float>(), t.b.as<float>(), t.c.as<float>()});
  }

  std::vector<AABB<float>> aabbs;
  aabbs.reserve(tris_float.size());
  for (const auto &t : tris_float) {
    aabbs.push_back(t.calc_aabb());
  }

  auto bvh = build_bvh(aabbs);
  const auto &root = bvh.nodes[0];
  const auto &aabb = root.aabb;

  std::uniform_real_distribution<float> dist_x(aabb.min.x, aabb.max.x);
  std::uniform_real_distribution<float> dist_y(aabb.min.y, aabb.max.y);
  std::uniform_real_distribution<float> dist_z(aabb.min.z, aabb.max.z);

  std::vector<Vec3<double>> samples;
  samples.reserve(num_samples);

  for (size_t i = 0; i < num_samples; i++) {
    auto sample_x = dist_x(rng);
    auto sample_y = dist_y(rng);
    auto sample_z = dist_z(rng);
    Vec3<float> sample(sample_x, sample_y, sample_z);
    size_t num_odd = 0;
    for (const auto &dir : directions) {
      Ray<float> r(sample, dir);
      auto num_intersections = bvh.count_intersections(r, tris_float);
      if (num_intersections % 2 == 1) {
        num_odd++;
      }
    }
    if (num_odd > (directions.size() / 2)) {
      samples.push_back(sample.as<double>());
    }
  }

  write_points_to_ply(output_path, samples);
}