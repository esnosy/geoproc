#include <iostream>
#include <chrono>
#include <string>
#include <vector>

#include "../libs/read_stl.hpp"

struct AABB
{
    vec3 upper, lower;
};

struct Node
{
    AABB aabb;
    Node *left, *right;
    size_t first, count;
};

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

    std::vector<AABB> aabbs(vertices.size() / 3);
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

    auto root = new Node;
    root->left = nullptr;
    root->right = nullptr;
    root->first = 0;
    root->count = aabbs.size();

    std::vector<Node *> stack;
    stack.push_back(root);

    while (!stack.empty())
    {
        auto node = stack.back();
        stack.pop_back();

        node->aabb = aabbs[0];
        for (size_t i = node->first; i < node->count; i++)
        {
            node->aabb.lower = aabbs[i].lower.min(node->aabb.lower);
            node->aabb.upper = aabbs[i].upper.max(node->aabb.upper);
        }

        if (node->count < 2)
            continue;

        auto left = new Node;
        left->left = nullptr;
        left->right = nullptr;
        left->first = node->first;
        left->count = node->count / 2;

        auto right = new Node;
        right->left = nullptr;
        right->right = nullptr;
        right->first = left->first + left->count;
        right->count = node->count - left->count;

        node->left = left;
        node->right = right;

        stack.push_back(left);
        stack.push_back(right);
    }

    std::cout << "FOOOOOOOOOOO" << std::endl;
}