#include <iostream>
#include <chrono>
#include <string>
#include <vector>

#include "../libs/read_stl.hpp"
#include "../libs/bvh.hpp"

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cerr << "Expected arguments: /path/to/input.stl"
                  << std::endl;
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
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        auto a = vertices[i];
        auto b = vertices[i + 1];
        auto c = vertices[i + 2];
        AABB aabb;
        aabb.upper = a.max(b).max(c);
        aabb.lower = a.min(b).min(c);
        aabbs.push_back(aabb);
    }

    Node *root = build_bvh(aabbs);

    size_t num_nodes = 0;
    std::vector<Node *> stack;

    stack.push_back(root);
    while (!stack.empty())
    {
        auto node = stack.back();
        stack.pop_back();

        num_nodes++;

        if (node->left)
            stack.push_back(node->left);
        if (node->right)
            stack.push_back(node->right);
    }

    delete_tree(root);

    std::cout << "Num nodes: " << num_nodes << std::endl;
    std::cout << "Num primitives: " << aabbs.size() << std::endl;
    std::cout << "Predicted num nodes: " << 2 * aabbs.size() - 1 << std::endl;
    std::cout << ((2 * aabbs.size() - 1) == num_nodes) << std::endl;
}