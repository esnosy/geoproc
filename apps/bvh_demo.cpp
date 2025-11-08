#include <chrono>
#include <iostream>
#include <vector>

#include "../libs/bvh.hpp"
#include "../libs/read_stl.hpp"

size_t max_primitives_count(BVH_Node *root) {
  size_t result = 0;
  std::vector<BVH_Node *> stack;
  stack.push_back(root);
  while (!stack.empty()) {
    auto node = stack.back();
    stack.pop_back();

    result = std::max(result, node->count);

    if (node->left)
      stack.push_back(node->left);
    if (node->right)
      stack.push_back(node->right);
  }
  return result;
}

size_t count_nodes(BVH_Node *root) {
  size_t num_nodes = 0;
  std::vector<BVH_Node *> stack;
  stack.push_back(root);
  while (!stack.empty()) {
    auto node = stack.back();
    stack.pop_back();

    num_nodes++;

    if (node->left)
      stack.push_back(node->left);
    if (node->right)
      stack.push_back(node->right);
  }
  return num_nodes;
}

int main(int argc, char *argv[]) {

  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/input.stl" << std::endl;
    return 1;
  }

  const char *input_path = argv[1];

  auto t0 = std::chrono::high_resolution_clock::now();
  auto vertices = read_stl(input_path);
  auto t1 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> ms = t1 - t0;
  std::cout << "Read " << vertices.size() << " vertices in " << ms.count()
            << "ms" << std::endl;

  std::vector<AABB> aabbs;
  aabbs.reserve(vertices.size() / 3);
  for (size_t i = 0; i < vertices.size(); i += 3) {
    auto a = vertices[i];
    auto b = vertices[i + 1];
    auto c = vertices[i + 2];
    AABB aabb;
    aabb.upper = a.max(b).max(c);
    aabb.lower = a.min(b).min(c);
    aabbs.push_back(aabb);
  }

  BVH_Node *root = build_bvh(aabbs);
  size_t num_nodes = count_nodes(root);

  std::cout << "Num nodes: " << num_nodes << std::endl;
  std::cout << "Num primitives: " << aabbs.size() << std::endl;
  std::cout << "Predicted num nodes: " << 2 * aabbs.size() - 1 << std::endl;
  std::cout << ((2 * aabbs.size() - 1) == num_nodes) << std::endl;

  std::cout << "Max primitives count: " << max_primitives_count(root)
            << std::endl;

  delete_tree(root);
}