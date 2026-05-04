#pragma once

#include <fstream>
#include <limits>
#include <string>

#include "indexed_ngon_mesh.hpp"

template <typename T> Indexed_Ngon_Mesh<T> read_off(const char *path) {
  std::ifstream ifs(path, std::ios::binary);
  std::string token;
  ifs >> token; // expecting "OFF"
  size_t num_vertices, num_faces, num_edges;
  ifs >> num_vertices >> num_faces >> num_edges;

  Indexed_Ngon_Mesh<T> result;
  result.vertices.reserve(num_vertices);
  result.faces.resize(num_faces);
  for (size_t i = 0; i < num_vertices; i++) {
    Vec3<T> v(0, 0, 0);
    ifs >> v.x >> v.y >> v.z;
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    result.vertices.push_back(v);
  }
  for (size_t i = 0; i < num_faces; i++) {
    size_t num_face_vertices;
    ifs >> num_face_vertices;
    result.faces[i].resize(num_face_vertices);
    for (size_t j = 0; j < num_face_vertices; j++) {
      size_t vertex_index;
      ifs >> vertex_index;
      result.faces[i][j] = vertex_index;
    }
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }
  return result;
}