#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "../libs/aabb.hpp"
#include "../libs/bvh.hpp"
#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/stl_io.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/input.stl" << std::endl;
    return 1;
  }

  const char *input_path = argv[1];

  auto t0 = std::chrono::high_resolution_clock::now();
  auto tris = read_stl(input_path);
  auto t1 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration<double, std::milli>(t1 - t0);
  std::cout << "Read " << tris.size() << " triangles in " << duration.count()
            << " ms" << std::endl;

  auto mesh = Indexed_Tri_Mesh<double>::from_stl_tris(tris);
  auto vertex_normals = mesh.calc_vertex_normals();

  std::vector<Triangle<float>> tris_float;
  tris_float.reserve(tris.size());
  for (const auto &t : tris) {
    tris_float.push_back({t.a.as<float>(), t.b.as<float>(), t.c.as<float>()});
  }

  std::vector<AABB<float>> aabbs;
  aabbs.reserve(tris_float.size());
  for (const auto &t : tris_float) {
    const auto &aabb = t.calc_aabb();
    aabbs.push_back(aabb);
  }

  t0 = std::chrono::high_resolution_clock::now();
  auto bvh = build_bvh(aabbs);
  t1 = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration<double, std::milli>(t1 - t0);
  std::cout << "Built BVH in " << duration.count() << " ms" << std::endl;

  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 1920;

  // Calculate the image height, and ensure that it's at least 1.
  int image_height = int(image_width / aspect_ratio);
  image_height = (image_height < 1) ? 1 : image_height;

  // Camera

  auto focal_length = 1.0;
  auto viewport_height = 2.0;
  auto viewport_width = viewport_height * (double(image_width) / image_height);
  auto camera_center = Vec3<double>(0, 0, 0);

  // Calculate the vectors across the horizontal and down the vertical viewport
  // edges.
  auto viewport_u = Vec3<double>(viewport_width, 0, 0);
  auto viewport_v = Vec3<double>(0, -viewport_height, 0);

  // Calculate the horizontal and vertical delta vectors from pixel to pixel.
  auto pixel_delta_u = viewport_u / image_width;
  auto pixel_delta_v = viewport_v / image_height;

  // Calculate the location of the upper left pixel.
  auto viewport_upper_left = camera_center - Vec3<double>(0, 0, focal_length) -
                             viewport_u / 2 - viewport_v / 2;
  auto pixel00_loc =
      viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

  std::ofstream ofs("output.ppm", std::ios::binary | std::ios::trunc);
  ofs << "P6\n" << image_width << " " << image_height << "\n255\n";

  t0 = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < image_height; j++) {
    for (int i = 0; i < image_width; i++) {
      auto pixel_center = pixel00_loc + (double(i) * pixel_delta_u) +
                          (double(j) * pixel_delta_v);
      auto ray_direction = pixel_center - camera_center;
      Ray<double> ray(camera_center, ray_direction);
      auto result = bvh.intersect_tris(ray, tris);
      if (result.intersection.hit) {
        const auto &tri = mesh.tris[result.tri_idx];
        const auto &n1 = vertex_normals[tri[0]];
        const auto &n2 = vertex_normals[tri[1]];
        const auto &n3 = vertex_normals[tri[2]];
        auto u = result.intersection.u;
        auto v = result.intersection.v;
        auto normal = (1 - u - v) * n1 + u * n2 + v * n3;
        uint32_t c = std::clamp(
            std::abs(normal.dot(-ray.direction.normalized())) * 255.0, 0.0,
            255.0);

        uint8_t color[3] = {uint8_t(c), uint8_t(c), uint8_t(c)};
        ofs.write(reinterpret_cast<char *>(color), 3);
      } else {
        uint8_t color[3] = {0, 0, 0};
        ofs.write(reinterpret_cast<char *>(color), 3);
      }
    }
  }
  t1 = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration<double, std::milli>(t1 - t0);
  std::cout << "Rendered image in " << duration.count() << " ms" << std::endl;

  uint32_t node_count = 0;
  uint32_t num_leaf_nodes = 0;
  uint32_t max_prim_count = 0;

  std::stack<BVH_Node *> stack;
  stack.push(bvh.nodes);
  while (!stack.empty()) {
    auto node = stack.top();
    stack.pop();
    if (node->is_leaf()) {
      max_prim_count = std::max(max_prim_count, node->prim_count);
      num_leaf_nodes++;
    } else {
      stack.push(bvh.nodes + node->left_first);
      stack.push(bvh.nodes + node->left_first + 1);
    }
    node_count++;
  }

  bvh.free();

  std::cout << "Node count: " << node_count << std::endl;
  std::cout << "Max prim. count: " << max_prim_count << std::endl;
  std::cout << "Num. leaf nodes: " << num_leaf_nodes << std::endl;
}