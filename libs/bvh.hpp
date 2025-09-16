#pragma once

#include <vector>

#include "vec3.hpp"

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

Node *build_bvh(const std::vector<AABB> &aabbs);
void delete_tree(Node *node);