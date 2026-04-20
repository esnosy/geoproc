#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "fast_float.hpp"
#include "stl_io.hpp"

struct File_Buf {
  char *buf;
  size_t size;
  size_t offset;

  bool compare_token(const char *token, size_t token_size) {
    if (offset + token_size > size)
      return false;
    for (size_t i = 0; i < token_size; i++) {
      if (buf[offset + i] != token[i]) {
        return false;
      }
    }
    return true;
  }
  void skip_spaces() {
    while (offset < size && std::isspace(buf[offset])) {
      ++offset;
    }
  }
  void skip_token() {
    while (offset < size && !std::isspace(buf[offset])) {
      ++offset;
    }
  }
  void read_vertex(Vec3<double> &output) {
    for (int i = 0; i < 3; i++) {
      fast_float::from_chars(buf + offset, buf + size, output[i]);
      skip_token();
      skip_spaces();
    }
  }
};

std::vector<Triangle<double>> read_stl(const char *path) {
  std::vector<Triangle<double>> tris;

  std::ifstream ifs(path, std::ios::binary);

  ifs.seekg(80, std::ios::beg);
  uint32_t num_tris;
  ifs.read(reinterpret_cast<char *>(&num_tris), sizeof(uint32_t));

  uint64_t expected_size = 50 * uint64_t(num_tris) + 84;

  ifs.seekg(0, std::ios::end);
  auto file_size = ifs.tellg();

  if (file_size == expected_size) {
    ifs.seekg(84, std::ios::beg);
    tris.reserve(num_tris);
    for (uint32_t i = 0; i < num_tris; i++) {
      Triangle<double> t = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
      float normal[3];
      ifs.read(reinterpret_cast<char *>(normal), sizeof(float[3]));
      for (int j = 0; j < 3; j++) {
        float buf[3];
        ifs.read(reinterpret_cast<char *>(buf), sizeof(float[3]));
        t[j] = Vec3<double>(buf);
      }
      uint16_t attribute_byte_count;
      ifs.read(reinterpret_cast<char *>(&attribute_byte_count),
               sizeof(uint16_t));
      tris.push_back(t);
    }
  } else {
    ifs.seekg(0, std::ios::beg);

    char *buf = (char *)malloc(file_size);
    ifs.read(buf, file_size);
    File_Buf file_buf{buf, (size_t)file_size, 0};

    while (file_buf.offset < file_buf.size) {
      file_buf.skip_spaces();
      if (file_buf.compare_token("vertex", 6)) {
        Triangle<double> t = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        file_buf.skip_token();  // Skip "vertex"
        file_buf.skip_spaces(); // Skip spaces after "vertex"
        file_buf.read_vertex(t.a);

        assert(file_buf.compare_token("vertex", 6));
        file_buf.skip_token();  // Skip "vertex"
        file_buf.skip_spaces(); // Skip spaces after "vertex"
        file_buf.read_vertex(t.b);

        assert(file_buf.compare_token("vertex", 6));
        file_buf.skip_token();  // Skip "vertex"
        file_buf.skip_spaces(); // Skip spaces after "vertex"
        file_buf.read_vertex(t.c);

        tris.push_back(t);
      } else {
        file_buf.skip_token();
      }
    }

    free(buf);
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
