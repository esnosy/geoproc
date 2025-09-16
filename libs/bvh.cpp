#include <vector>

#include "bvh.hpp"
#include "vec3.hpp"

// Thanks to: https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
Node *build_bvh(const std::vector<AABB> &aabbs)
{
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
            size_t remaped_index = indices[i];
            auto center = aabbs[remaped_index].calc_center_axis(split_axis);
            if (center < split_value)
            {
                left_partition_buf.push_back(remaped_index);
            }
            else
            {
                right_partition_buf.push_back(remaped_index);
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
    return root;
}

void delete_tree(Node *node)
{
    if (!node)
        return;
    delete_tree(node->left);
    delete_tree(node->right);
    delete node;
}