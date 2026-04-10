#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <stack>
#include <vector>

#include "../libs/aabb.hpp"
#include "../libs/stl_io.hpp"

struct BVHNode {
  AABB aabb;
  BVHNode *left;
  BVHNode *right;
  size_t first;
  size_t count;
};

struct Ray {
  Vec3 origin;
  Vec3 direction;
  Ray(const Vec3 &origin, const Vec3 &direction)
      : origin(origin), direction(direction) {}
};

bool does_ray_intersect_aabb(const Ray &ray, const AABB &aabb) {
  double min = 0.0;
  double max = std::numeric_limits<double>::infinity();
  for (int i = 0; i < 3; i++) {
    if (std::abs(ray.direction[i]) < 0.00001) {
      if (ray.origin[i] > aabb.max[i] || ray.origin[i] < aabb.min[i]) {
        return false;
      }
    } else {
      double t_max = (aabb.max[i] - ray.origin[i]) / ray.direction[i];
      double t_min = (aabb.min[i] - ray.origin[i]) / ray.direction[i];
      if (ray.direction[i] < 0.0) {
        std::swap(t_max, t_min);
      }
      min = std::max(t_min, min);
      max = std::min(t_max, max);
      if (max < min) {
        return false;
      }
    }
  }
  return true;
}

bool does_ray_intersect_bvh(const Ray &ray, const BVHNode *root) {
  std::stack<const BVHNode *> stack;
  stack.push(root);
  while (!stack.empty()) {
    const BVHNode *node = stack.top();
    stack.pop();
    if (!does_ray_intersect_aabb(ray, node->aabb)) {
      continue;
    }
    if (node->left) {
      stack.push(node->left);
    }
    if (node->right) {
      stack.push(node->right);
    }
    if (node->left == nullptr && node->right == nullptr) {
      return true;
    }
  }
  return false;
}

BVHNode *build_bvh(std::vector<AABB> &aabbs) {
  auto root = new BVHNode;
  root->left = nullptr;
  root->right = nullptr;
  root->first = 0;
  root->count = aabbs.size();

  std::stack<BVHNode *> stack;
  stack.push(root);

  while (!stack.empty()) {
    BVHNode *node = stack.top();
    stack.pop();
    Vec3 mean_of_squares(0.0), mean(0.0);
    node->aabb = AABB(std::numeric_limits<double>::infinity(),
                      -std::numeric_limits<double>::infinity());
    for (size_t i = node->first; i < node->first + node->count; i++) {
      node->aabb = node->aabb.join(aabbs[i]);
      Vec3 center = aabbs[i].calc_center();
      mean_of_squares += center * center;
      mean += center;
    }
    if (node->count < 2) {
      continue;
    }
    mean_of_squares /= node->count;
    mean /= node->count;
    Vec3 variance = mean_of_squares - mean * mean;
    int split_axis = 0;
    if (variance[1] > variance[split_axis]) {
      split_axis = 1;
    }
    if (variance[2] > variance[split_axis]) {
      split_axis = 2;
    }
    double split_value = mean[split_axis];
    auto first_element_of_second_group = std::partition(
        aabbs.begin() + node->first, aabbs.begin() + node->first + node->count,
        [split_axis, split_value](const AABB &aabb) {
          return aabb.calc_axis_center(split_axis) < split_value;
        });

    size_t left_count = std::distance(aabbs.begin() + node->first,
                                      first_element_of_second_group);
    size_t right_count = node->count - left_count;

    if (left_count == 0 || right_count == 0) {
      continue;
    }

    auto left = new BVHNode;
    left->left = nullptr;
    left->right = nullptr;
    left->first = node->first;
    left->count = left_count;

    auto right = new BVHNode;
    right->left = nullptr;
    right->right = nullptr;
    right->first = left->first + left->count;
    right->count = right_count;

    node->left = left;
    node->right = right;

    stack.push(left);
    stack.push(right);
  }
  return root;
}

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

  std::vector<AABB> aabbs;
  aabbs.reserve(tris.size());
  for (const auto &tri : tris) {
    aabbs.push_back(tri.calc_aabb());
  }

  auto root = build_bvh(aabbs);

  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 1920;

  // Calculate the image height, and ensure that it's at least 1.
  int image_height = int(image_width / aspect_ratio);
  image_height = (image_height < 1) ? 1 : image_height;

  // Camera

  auto focal_length = 1.0;
  auto viewport_height = 2.0;
  auto viewport_width = viewport_height * (double(image_width) / image_height);
  auto camera_center = Vec3(0, 0, 0);

  // Calculate the vectors across the horizontal and down the vertical viewport
  // edges.
  auto viewport_u = Vec3(viewport_width, 0, 0);
  auto viewport_v = Vec3(0, -viewport_height, 0);

  // Calculate the horizontal and vertical delta vectors from pixel to pixel.
  auto pixel_delta_u = viewport_u / image_width;
  auto pixel_delta_v = viewport_v / image_height;

  // Calculate the location of the upper left pixel.
  auto viewport_upper_left = camera_center - Vec3(0, 0, focal_length) -
                             viewport_u / 2 - viewport_v / 2;
  auto pixel00_loc =
      viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

  std::ofstream ofs("output.ppm", std::ios::binary);
  ofs << "P3\n" << image_width << " " << image_height << "\n255\n";

  for (int j = 0; j < image_height; j++) {
    for (int i = 0; i < image_width; i++) {
      auto pixel_center =
          pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
      auto ray_direction = pixel_center - camera_center;
      Ray ray(camera_center, ray_direction);
      if (does_ray_intersect_bvh(ray, root)) {
        ofs << "255 255 255\n";
      } else {
        ofs << "0 0 0\n";
      }
    }
  }

  size_t node_count = 0;
  size_t num_leaf_nodes = 0;
  size_t max_prim_count = 0;

  std::stack<BVHNode *> stack;
  stack.push(root);
  while (!stack.empty()) {
    auto node = stack.top();
    stack.pop();
    if (node->left)
      stack.push(node->left);
    if (node->right)
      stack.push(node->right);
    if (node->right == nullptr && node->left == nullptr) {
      max_prim_count = std::max(max_prim_count, node->count);
      num_leaf_nodes++;
    }
    delete node;
    node_count++;
  }

  std::cout << "Node count: " << node_count << std::endl;
  std::cout << "Max prim. count: " << max_prim_count << std::endl;
  std::cout << "Num. leaf nodes: " << num_leaf_nodes << std::endl;
}