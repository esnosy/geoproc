#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "fast_float.hpp"
#include "stl_io.hpp"

static void skip_spaces(char *&p) {
  while (isspace(*p) && *p) {
    p++;
  }
}

static void read_vertex(char *start, char *end, Vec3 &output) {
  skip_spaces(start);
  auto res = fast_float::from_chars(start, end, output.x);
  start = const_cast<char *>(res.ptr);
  skip_spaces(start);
  res = fast_float::from_chars(start, end, output.y);
  start = const_cast<char *>(res.ptr);
  skip_spaces(start);
  res = fast_float::from_chars(start, end, output.z);
}

std::vector<Triangle> read_stl(const char *path) {
  std::vector<Triangle> tris;

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
      Triangle t;
      float normal[3];
      fread(normal, sizeof(float[3]), 1, file);
      for (int j = 0; j < 3; j++) {
        float buf[3];
        fread(buf, sizeof(float[3]), 1, file);
        t[j] = Vec3(buf);
      }
      uint16_t attribute_byte_count;
      fread(&attribute_byte_count, sizeof(uint16_t), 1, file);
      tris.push_back(t);
    }
  } else {
    fseek(file, 0, SEEK_SET);
    char line[256];
    while (fgets(line, 256, file)) {
      char *p = line;
      skip_spaces(p);
      if (memcmp(p, "vertex", 6) == 0) {
        p += 6; // Skip "vertex"

        Triangle t;

        read_vertex(p, line + 256, t.a);

        fgets(line, 256, file);
        p = line;
        skip_spaces(p);
        p += 6; // Skip "vertex"

        read_vertex(p, line + 256, t.b);

        fgets(line, 256, file);
        p = line;
        skip_spaces(p);
        p += 6; // Skip "vertex"

        read_vertex(p, line + 256, t.c);

        tris.push_back(t);
      }
    }
  }

  fclose(file);
  return tris;
}

void write_stl_binary(const char *path, const std::vector<Triangle> &tris) {
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

void write_stl_ascii(const char *path, const std::vector<Triangle> &tris) {
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
