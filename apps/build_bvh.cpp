#include <iostream>
#include <vector>

#include "../libs/aabb.hpp"
#include "../libs/read_stl.hpp"

template <typename T, size_t N> struct Node {
  AABB<T, N> aabb;
  Node *left, *right;
  size_t first, count;
};

using Node3f = Node<float, 3>;

void free_tree(Node3f *root) {
  std::vector<Node3f *> stack;
  stack.push_back(root);
  while (!stack.empty()) {
    auto node = stack.back();
    stack.pop_back();
    if (node->left && node->right) {
      stack.push_back(node->left);
      stack.push_back(node->right);
    }
    delete node;
  }
}

int main(int argc, char **argv) {

  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/mesh.stl" << std::endl;
    return 1;
  }
  const char *input_filepath = argv[1];
  auto vertices = read_stl(input_filepath);

  auto *root = new Node3f;
  root->left = nullptr;
  root->right = nullptr;
  root->first = 0;
  root->count = vertices.size();

  std::vector<Node3f *> stack;
  stack.push_back(root);

  std::vector<Vec3f> left_buf;
  std::vector<Vec3f> right_buf;

  while (!stack.empty()) {
    Node3f *node = stack.back();
    stack.pop_back();
    node->aabb.max = node->aabb.min = vertices[node->first];
    for (size_t i = (node->first + 1); i < (node->first + node->count); i++) {
      node->aabb.max = vertices[i].element_wise_max(node->aabb.max);
      node->aabb.min = vertices[i].element_wise_min(node->aabb.min);
    }
    if (node->count < 2)
      continue;

    auto dims = node->aabb.calc_dims();
    size_t split_axis = 0;
    if (dims[1] > dims[split_axis])
      split_axis = 1;
    if (dims[2] > dims[split_axis])
      split_axis = 2;

    float split_value =
        0.5f * node->aabb.max[split_axis] + 0.5f * node->aabb.min[split_axis];

    // Partition
    left_buf.clear();
    right_buf.clear();
    for (size_t i = node->first; i < (node->first + node->count); i++) {
      if (vertices[i][split_axis] < split_value) {
        left_buf.push_back(vertices[i]);
      } else {
        right_buf.push_back(vertices[i]);
      }
    }

    if (left_buf.empty() || right_buf.empty())
      continue;
    vertices.clear();
    for (auto v : left_buf)
      vertices.push_back(v);
    for (auto v : right_buf)
      vertices.push_back(v);

    auto *left = new Node3f;
    left->left = nullptr;
    left->right = nullptr;
    left->first = node->first;
    left->count = left_buf.size();

    auto *right = new Node3f;
    right->left = nullptr;
    right->right = nullptr;
    right->first = left->first + left->count;
    right->count = right_buf.size();

    node->left = left;
    node->right = right;

    stack.push_back(left);
    stack.push_back(right);
  }

  free_tree(root);
  return 0;
}