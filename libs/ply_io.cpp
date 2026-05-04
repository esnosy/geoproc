#include <fstream>

#include "ply_io.hpp"

void write_points_to_ply(const char *path,
                         const std::vector<Vec3<double>> &points) {
  std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
  ofs << "ply\n"
      << "format binary_little_endian 1.0\n"
      << "element vertex " << points.size() << '\n'
      << "property double x\n"
      << "property double y\n"
      << "property double z\n"
      << "end_header\n";
  ofs.write(reinterpret_cast<const char *>(points.data()),
            points.size() * sizeof(Vec3<double>));
}