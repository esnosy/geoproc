#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
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