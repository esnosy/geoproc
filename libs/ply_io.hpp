#pragma once

#include <vector>

#include "vec3.hpp"

void write_points_to_ply(const char *path,
                         const std::vector<Vec3<double>> &points);