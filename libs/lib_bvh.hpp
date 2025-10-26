#pragma once

#include <vector>

#include "vec3.hpp"

struct AABB
{
    Vec3 upper, lower;
    AABB join(const AABB &rhs) const
    {
        return {upper.max(rhs.upper), lower.min(rhs.lower)};
    }
    float calc_center_axis(int i) const
    {
        return (upper[i] + lower[i]) * 0.5f;
    }
    Vec3 calc_extent() const
    {
        return upper - lower;
    }
};

struct BVH_Node
{
    AABB aabb;
    BVH_Node *left, *right;
    size_t first, count;
};

BVH_Node *build_bvh(const std::vector<AABB> &aabbs);
void delete_tree(BVH_Node *node);