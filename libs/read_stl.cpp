#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "vec3.hpp"

std::vector<vec3> read_stl(const char *path)
{

  std::ifstream ifs(path, std::ifstream::binary);

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

  std::vector<vec3> vertices;

  if (actual_file_size == expected_file_size)
  {
    vertices.reserve(possible_num_tris * 3);
    ifs.seekg(84, ifs.beg);
    for (uint32_t i = 0; i < possible_num_tris; i++)
    {
      ifs.seekg(12, ifs.cur); // Skip normal vector
      for (int j = 0; j < 3; j++)
      {
        vec3 vertex;
        ifs.read(reinterpret_cast<char *>(&vertex), sizeof(vec3));
        vertices.push_back(vertex);
      }
      ifs.seekg(2, ifs.cur); // Skip "attribute byte count"
    }
  }
  else
  {
    ifs.seekg(0, ifs.beg);
    std::string token;

    while (ifs >> token)
    {
      if (token == "vertex")
      {
        vec3 vertex;
        ifs >> vertex.x >> vertex.y >> vertex.z;
        vertices.push_back(vertex);
      }
    }
  }

  return vertices;
}