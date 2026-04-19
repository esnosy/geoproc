#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "fast_float.hpp"
#include "stl_io.hpp"

std::vector<Triangle<double>> read_stl(const char *path) {
  std::vector<Triangle<double>> tris;

  FILE *file = fopen(path, "rb");

  fseek(file, 80, SEEK_SET);
  uint32_t num_tris;
  fread(&num_tris, sizeof(uint32_t), 1, file);

  uint64_t expected_size = 50 * uint64_t(num_tris) + 84;

  fseek(file, 0, SEEK_END);
  auto file_size = ftell(file);

  if (file_size == expected_size) {
    fseek(file, 84, SEEK_SET);
    tris.reserve(num_tris);
    for (uint32_t i = 0; i < num_tris; i++) {
      Triangle<double> t;
      float normal[3];
      fread(normal, sizeof(float[3]), 1, file);
      for (int j = 0; j < 3; j++) {
        float buf[3];
        fread(buf, sizeof(float[3]), 1, file);
        t[j] = Vec3<double>(buf);
      }
      uint16_t attribute_byte_count;
      fread(&attribute_byte_count, sizeof(uint16_t), 1, file);
      tris.push_back(t);
    }
  } else {
    fseek(file, 0, SEEK_SET);

    char *file_buf = (char *)malloc(file_size);
    fread(file_buf, 1, file_size, file);

    size_t file_buf_offset = 0;

    auto compare_token = [&](const char *token, size_t token_size) {
      if (file_buf_offset + token_size > file_size)
        return false;
      for (size_t i = 0; i < token_size; i++) {
        if (file_buf[file_buf_offset + i] != token[i]) {
          return false;
        }
      }
      return true;
    };

    auto skip_spaces = [&]() {
      while (file_buf_offset < file_size &&
             std::isspace(file_buf[file_buf_offset])) {
        ++file_buf_offset;
      }
    };

    auto skip_token = [&]() {
      while (file_buf_offset < file_size &&
             !std::isspace(file_buf[file_buf_offset])) {
        ++file_buf_offset;
      }
    };

    auto read_vertex = [&](Vec3<double> &output) {
      for (int i = 0; i < 3; i++) {
        fast_float::from_chars(file_buf + file_buf_offset, file_buf + file_size,
                               output[i]);
        skip_token();
        skip_spaces();
      }
    };

    while (file_buf_offset < file_size) {
      skip_spaces();
      if (compare_token("vertex", 6)) {
        Triangle<double> t;
        skip_token();  // Skip "vertex"
        skip_spaces(); // Skip spaces after "vertex"
        read_vertex(t.a);

        assert(compare_token("vertex", 6));
        skip_token();  // Skip "vertex"
        skip_spaces(); // Skip spaces after "vertex"
        read_vertex(t.b);

        assert(compare_token("vertex", 6));
        skip_token();  // Skip "vertex"
        skip_spaces(); // Skip spaces after "vertex"
        read_vertex(t.c);

        tris.push_back(t);
      } else {
        skip_token();
      }
    }
  }

  return tris;
}

void write_stl_binary(const char *path,
                      const std::vector<Triangle<double>> &tris) {
  char header[80] = {};
  uint32_t num_tris = (uint32_t)tris.size();
  std::ofstream ofs(path, std::ios::binary);
  ofs.write(header, 80);
  ofs.write(reinterpret_cast<const char *>(&num_tris), sizeof(uint32_t));
  for (const auto &t : tris) {
    float normal[3] = {0, 0, 0};
    ofs.write(reinterpret_cast<const char *>(normal), sizeof(float[3]));
    for (int j = 0; j < 3; j++) {
      float buf[3] = {float(t[j].x), float(t[j].y), float(t[j].z)};
      ofs.write(reinterpret_cast<const char *>(buf), sizeof(float[3]));
    }
    uint16_t attribute_byte_count = 0;
    ofs.write(reinterpret_cast<const char *>(&attribute_byte_count),
              sizeof(uint16_t));
  }
}

void write_stl_ascii(const char *path,
                     const std::vector<Triangle<double>> &tris) {
  std::ofstream ofs(path, std::ios::binary);
  ofs << "solid \n";
  for (const auto &t : tris) {
    ofs << "  facet normal 0 0 0\n";
    ofs << "    outer loop\n";
    ofs << "      vertex " << t.a.x << " " << t.a.y << " " << t.a.z << "\n";
    ofs << "      vertex " << t.b.x << " " << t.b.y << " " << t.b.z << "\n";
    ofs << "      vertex " << t.c.x << " " << t.c.y << " " << t.c.z << "\n";
    ofs << "    endloop\n";
    ofs << "  endfacet\n";
  }
  ofs << "end solid \n";
}
