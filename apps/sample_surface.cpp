#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <utility>

#include "../libs/read_stl.hpp"
#include "../libs/vec3.hpp"

// https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#SamplingaTriangle
std::pair<float, float> uniform_sample_triangle(float u, float v)
{
  float su0 = std::sqrt(u);
  return std::pair<float, float>(1 - su0, v * su0);
}

int main(int argc, char *argv[])
{

  if (argc != 4)
  {
    std::cerr << "Expected arguments: /path/to/input.stl num_samples "
                 "/path/to/output.ply"
              << std::endl;
    return 1;
  }

  const char *input_path = argv[1];
  size_t num_samples = std::stoull(argv[2]);
  const char *output_path = argv[3];

  auto t0 = std::chrono::high_resolution_clock::now();
  auto vertices = read_stl(input_path);
  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> ms = t1 - t0;
  std::cout << "Read " << vertices.size() << " vertices in " << ms.count()
            << "ms" << std::endl;

  t0 = std::chrono::high_resolution_clock::now();

  std::ofstream ofs(output_path, std::ofstream::binary);
  ofs << "ply\n";
  ofs << "format binary_little_endian 1.0\n";
  ofs << "element vertex " << num_samples << "\n";
  ofs << "property float x\n";
  ofs << "property float y\n";
  ofs << "property float z\n";
  ofs << "end_header\n";

  std::vector<float> triangle_areas;
  triangle_areas.reserve(vertices.size() / 3);

  for (size_t ti = 0; ti < vertices.size(); ti += 3)
  {
    auto a = vertices[ti];
    auto b = vertices[ti + 1];
    auto c = vertices[ti + 2];
    auto ca = a - c;
    auto cb = b - c;
    float ta = ca.cross(cb).magnitude() * 0.5f;
    triangle_areas.push_back(ta);
  }

  std::default_random_engine random_engine;
  std::discrete_distribution<uint64_t> discrete_dist(triangle_areas.begin(),
                                                     triangle_areas.end());
  std::uniform_real_distribution<float> uniform_real_dist(0.0f, 1.0f);

  for (size_t i = 0; i < num_samples; i++)
  {
    uint64_t ti = discrete_dist(random_engine);
    auto a = vertices[ti * 3];
    auto b = vertices[ti * 3 + 1];
    auto c = vertices[ti * 3 + 2];
    auto ca = a - c;
    auto cb = b - c;

    float u = uniform_real_dist(random_engine);
    float v = uniform_real_dist(random_engine);
    auto [s, t] = uniform_sample_triangle(u, v);

    auto p = ca * s + cb * t + c;

    ofs.write(reinterpret_cast<char *>(&p), sizeof(vec3));
  }

  t1 = std::chrono::high_resolution_clock::now();
  ms = t1 - t0;
  std::cout << "Wrote " << num_samples << " samples in " << ms.count()
            << "ms" << std::endl;

  return 0;
}