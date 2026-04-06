#pragma once

#include <vector>

#include "triangle.hpp"

std::vector<Triangle> read_stl(const char *path);
void write_stl_binary(const char *path, const std::vector<Triangle> &tris);
void write_stl_ascii(const char *path, const std::vector<Triangle> &tris);