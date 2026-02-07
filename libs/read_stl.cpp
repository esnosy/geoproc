#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "vec3.hpp"

#define STARTS_WITH(lhs, rhs) std::strncmp(lhs, rhs, sizeof(rhs) - 1) == 0

static void skip_whitespace(char **p) {
  while (**p) {
    if (!std::isspace(**p))
      break;
    (*p)++;
  }
}

std::vector<Vec3> read_stl(const char *path) {
  std::FILE *f = std::fopen(path, "rb");

  // Assume binary STL and skip the 80 bytes header
  std::fseek(f, 80, SEEK_SET);
  uint32_t possible_num_tris;
  fread(&possible_num_tris, sizeof(uint32_t), 1, f);

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
  // STL ASCII format spec.
  // https://en.wikipedia.org/w/index.php?title=STL_(file_format)&oldid=1306712422#ASCII
  // foreach triangle
  // facet normal ni nj nk
  //     outer loop
  //         vertex v1x v1y v1z
  //         vertex v2x v2y v2z
  //         vertex v3x v3y v3z
  //     endloop
  // endfacet
  uint64_t expected_file_size = uint64_t(possible_num_tris) * 50 + 84;

  std::fseek(f, 0, SEEK_END);
  uint64_t actual_file_size = std::ftell(f);

  std::vector<Vec3> vertices;

  if (actual_file_size == expected_file_size) {
    vertices.reserve(possible_num_tris * 3);
    std::fseek(f, 84, SEEK_SET); // Skip header and triangle count
    for (uint32_t i = 0; i < possible_num_tris; i++) {
      std::fseek(f, 12, SEEK_CUR); // Skip normal vector
      for (int j = 0; j < 3; j++) {
        Vec3 vertex;
        std::fread(&vertex, sizeof(Vec3), 1, f);
        vertices.push_back(vertex);
      }
      std::fseek(f, 2, SEEK_CUR); // Skip "attribute byte count"
    }
  } else {
    std::fseek(f, 0, SEEK_SET);
    char line[256];
    while (std::fgets(line, 256, f) != nullptr) {
      char *p = line;
      char *end;
      skip_whitespace(&p);
      if (STARTS_WITH(p, "vertex")) {
        p += 6; // Skip "vertex"
        Vec3 vertex;
        errno = 0;
        vertex.x = std::strtof(p, &end);
        if (errno == ERANGE) {
          std::cerr << "STL: value out of range" << std::endl;
          break;
        }
        if (end == p && vertex.x == 0) {
          std::cerr << "STL: failed to convert to float" << std::endl;
          break;
        }
        p = end;
        errno = 0;
        vertex.y = std::strtof(p, &end);
        if (errno == ERANGE) {
          std::cerr << "STL: value out of range" << std::endl;
          break;
        }
        if (end == p && vertex.y == 0) {
          std::cerr << "STL: failed to convert to float" << std::endl;
          break;
        }
        p = end;
        errno = 0;
        vertex.z = std::strtof(p, &end);
        if (errno == ERANGE) {
          std::cerr << "STL: value out of range" << std::endl;
          break;
        }
        if (end == p && vertex.z == 0) {
          std::cerr << "STL: failed to convert to float" << std::endl;
          break;
        }
        vertices.push_back(vertex);
      }
    }
  }

  return vertices;
}