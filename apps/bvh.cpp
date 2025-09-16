#include <iostream>
#include <chrono>
#include <string>
#include <vector>

#include "../libs/read_stl.hpp"

struct AABB
{
    vec3 upper, lower;
    AABB join(const AABB &rhs) const
    {
        return {
            upper.max(rhs.upper),
            lower.min(rhs.lower)};
    }
    float calc_center_axis(int i) const
    {
        return (upper[i] + lower[i]) * 0.5f;
    }
    vec3 calc_extent() const { return upper - lower; }
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

    auto root = new Node;
    root->left = nullptr;
    root->right = nullptr;
    root->first = 0;
    root->count = aabbs.size();

    std::vector<Node *> stack;
    stack.push_back(root);

    std::vector<size_t> left_partition_buf;
    std::vector<size_t> right_partition_buf;

    left_partition_buf.reserve(aabbs.size());
    right_partition_buf.reserve(aabbs.size());

    std::vector<size_t> indices;
    indices.reserve(aabbs.size());
    for (size_t i = 0; i < aabbs.size(); i++)
    {
        indices.push_back(i);
    }

    while (!stack.empty())
    {
        auto node = stack.back();
        stack.pop_back();

        node->aabb = aabbs[indices[node->first]];
        for (size_t i = node->first + 1; i < node->first + node->count; i++)
        {
            node->aabb = node->aabb.join(aabbs[indices[i]]);
        }

        if (node->count < 2)
            continue;

        vec3 extent = node->aabb.calc_extent();

        int split_axis = 0;
        if (extent[1] > extent[split_axis])
            split_axis = 1;
        if (extent[2] > extent[split_axis])
            split_axis = 2;

        float split_value = node->aabb.calc_center_axis(split_axis);

        for (size_t i = node->first + 1; i < node->first + node->count; i++)
        {
            auto center = aabbs[indices[i]].calc_center_axis(split_axis);
            if (center < split_value)
            {
                left_partition_buf.push_back(indices[i]);
            }
            else
            {
                right_partition_buf.push_back(indices[i]);
            }
        }

        if (left_partition_buf.empty() || right_partition_buf.empty())
            continue;

        auto left = new Node;
        left->left = nullptr;
        left->right = nullptr;
        left->first = node->first;
        left->count = left_partition_buf.size();

        left_partition_buf.clear();
        right_partition_buf.clear();

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

    size_t num_nodes = 0;

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

    std::cout << "Num nodes: " << num_nodes << std::endl;
    std::cout << "Num primitives: " << aabbs.size() << std::endl;
    std::cout << "Predicted num nodes: " << 2 * aabbs.size() - 1 << std::endl;
    std::cout << ((2 * aabbs.size() - 1) == num_nodes) << std::endl;
}