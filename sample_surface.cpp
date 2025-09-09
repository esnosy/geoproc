#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

struct vec3 {
  float x, y, z;
  vec3 operator+(const vec3 &rhs) const {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  vec3 operator-(const vec3 &rhs) const {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }
  vec3 cross(const vec3 &rhs) const {
    return {y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x};
  }
  vec3 operator*(const float &rhs) const { return {x * rhs, y * rhs, z * rhs}; }
  float magnitude() const { return sqrt(x * x + y * y + z * z); }
};

// https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations#SamplingaTriangle
std::pair<float, float> uniform_sample_triangle(float u, float v) {
  float su0 = std::sqrt(u);
  return std::pair<float, float>(1 - su0, v * su0);
}

int main(int argc, char *argv[]) {

  if (argc != 4) {
    std::cerr << "Expected arguments: /path/to/input.stl num_samples "
                 "/path/to/output.ply"
              << std::endl;
    return 1;
  }

  const char *input_path = argv[1];
  size_t num_samples = std::stoull(argv[2]);
  const char *output_path = argv[3];

  std::ifstream ifs(input_path, std::ifstream::binary);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open file" << std::endl;
    return 1;
  }

  std::vector<vec3> vertices;

  // Assume binary STL and skip the 80 bytes header
  ifs.seekg(80, ifs.beg);
  uint32_t possible_num_tris;
  ifs.read(reinterpret_cast<char *>(&possible_num_tris), sizeof(uint32_t));

  // STL binary format spec.
  // https://en.wikipedia.org/w/index.php?title=STL_(file_format)&oldid=1306712422#Binary
  // UINT8[80]    – Header                 - 80 bytes
  // UINT32       – Number of triangles    - 04 bytes
  // foreach triangle                      - 50 bytes
  //     REAL32[3] – Normal vector         - 12 bytes
  //     REAL32[3] – Vertex 1              - 12 bytes
  //     REAL32[3] – Vertex 2              - 12 bytes
  //     REAL32[3] – Vertex 3              - 12 bytes
  //     UINT16    – Attribute byte count  - 02 bytes
  // end
  uint64_t expected_file_size = uint64_t(possible_num_tris) * 50 + 84;

  ifs.seekg(0, ifs.end);
  uint64_t actual_file_size = ifs.tellg();

  if (actual_file_size == expected_file_size) {
    vertices.reserve(possible_num_tris * 3);
    ifs.seekg(84, ifs.beg);
    for (uint32_t i = 0; i < possible_num_tris; i++) {
      ifs.seekg(12, ifs.cur); // Skip normal vector
      for (int j = 0; j < 3; j++) {
        vec3 vertex;
        ifs.read(reinterpret_cast<char *>(&vertex), sizeof(vec3));
        vertices.push_back(vertex);
      }
      ifs.seekg(2, ifs.cur); // Skip attribute byte count
    }
  } else {
    ifs.seekg(0, ifs.beg);
    std::string token;

    while (ifs >> token) {
      if (token == "vertex") {
        vec3 vertex;
        ifs >> vertex.x >> vertex.y >> vertex.z;
        vertices.push_back(vertex);
      }
    }
  }
  std::cout << vertices.size() << std::endl;

  std::ofstream ofs(output_path, std::ofstream::binary);
  ofs << "ply\n";
  ofs << "format binary_little_endian 1.0\n";
  ofs << "element vertex " << num_samples << "\n";
  ofs << "property float x\n";
  ofs << "property float y\n";
  ofs << "property float z\n";
  ofs << "end_header\n";

  std::vector<float> triangle_areas;

  for (size_t ti = 0; ti < vertices.size(); ti += 3) {
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

  for (size_t i = 0; i < num_samples; i++) {
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

  return 0;
}