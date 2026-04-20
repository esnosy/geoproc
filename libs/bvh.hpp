#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "aabb.hpp"
#include "triangle.hpp"
#include "vec3.hpp"

#ifndef _WIN32
#define _aligned_malloc(size, alignment) std::aligned_alloc(alignment, size)
#define _aligned_free(ptr) std::free(ptr)
#else
#include <malloc.h>
#endif

struct BVH_Node {
  AABB<float> aabb;
  uint32_t left_first, prim_count;
  bool is_leaf() { return prim_count > 0; }
};

template <typename T> struct Ray_Triangle_Intersection {
  T t, u, v;
  bool hit = false;
};

template <typename T> struct Ray_Triangles_Intersection {
  Ray_Triangle_Intersection<T> intersection;
  uint32_t tri_idx;
};

template <typename T> struct Ray {
  Vec3<T> origin;
  Vec3<T> direction;
  Ray(const Vec3<T> &origin, const Vec3<T> &direction)
      : origin(origin), direction(direction) {}
};

template <typename T>
Ray_Triangle_Intersection<T> intersect_ray_triangle(Ray<T> &ray,
                                                    const Triangle<T> &tri) {
  Ray_Triangle_Intersection<T> result;
  const Vec3<T> edge1 = tri.b - tri.a;
  const Vec3<T> edge2 = tri.c - tri.a;
  const Vec3<T> h = ray.direction.cross(edge2);
  const T a = edge1.dot(h);
  if (a > -0.000001f && a < 0.000001f)
    return result; // ray parallel to triangle
  const T f = 1 / a;
  const Vec3<T> s = ray.origin - tri.a;
  const T u = f * s.dot(h);
  if (u < 0 || u > 1)
    return result;
  const Vec3<T> q = s.cross(edge1);
  const T v = f * ray.direction.dot(q);
  if (v < 0 || u + v > 1)
    return result;
  const T t = f * edge2.dot(q);
  if (t > 0.000001f) {
    result.hit = true;
    result.t = t;
    result.u = u;
    result.v = v;
  }
  return result;
}

template <typename T>
bool intersect_ray_aabb(const Ray<T> &ray, const AABB<T> &aabb) {
  T min = 0.0;
  T max = std::numeric_limits<T>::infinity();
  for (int i = 0; i < 3; i++) {
    if (std::abs(ray.direction[i]) < 0.00001) {
      if (ray.origin[i] > aabb.max[i] || ray.origin[i] < aabb.min[i]) {
        return false;
      }
    } else {
      T t_max = (aabb.max[i] - ray.origin[i]) / ray.direction[i];
      T t_min = (aabb.min[i] - ray.origin[i]) / ray.direction[i];
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

struct BVH {
public:
  BVH_Node *nodes;
  std::vector<uint32_t> indices;

private:
  template <typename T>
  void intersect_tris(Ray<T> &ray, const std::vector<Triangle<T>> &tris,
                      uint32_t node_idx,
                      Ray_Triangles_Intersection<T> &tris_result) {
    BVH_Node &node = nodes[node_idx];
    AABB<T> node_aabb{node.aabb.min.as<T>(), node.aabb.max.as<T>()};
    if (!intersect_ray_aabb(ray, node_aabb)) {
      return;
    }
    if (node.is_leaf()) {
      for (uint32_t i = 0; i < node.prim_count; i++) {
        auto tri_idx = indices[node.left_first + i];
        auto tri_result = intersect_ray_triangle(ray, tris[tri_idx]);
        if (tri_result.hit) {
          if (tris_result.intersection.hit) {
            if (tri_result.t < tris_result.intersection.t) {
              tris_result.intersection = tri_result;
              tris_result.tri_idx = tri_idx;
            }
          } else {
            tris_result.intersection = tri_result;
            tris_result.tri_idx = tri_idx;
          }
        }
      }
    } else {
      intersect_tris(ray, tris, node.left_first, tris_result);
      intersect_tris(ray, tris, node.left_first + 1, tris_result);
    }
  }

public:
  void free() { _aligned_free(nodes); }

  template <typename T>
  Ray_Triangles_Intersection<T>
  intersect_tris(Ray<T> &ray, const std::vector<Triangle<T>> &tris) {
    Ray_Triangles_Intersection<T> tris_result;
    intersect_tris(ray, tris, 0, tris_result);
    return tris_result;
  }
};

BVH build_bvh(const std::vector<AABB<float>> &aabbs) {
  auto nodes =
      (BVH_Node *)_aligned_malloc(sizeof(BVH_Node) * 2 * aabbs.size(), 64);
  uint32_t root_node_idx = 0, nodes_used = 2;
  std::vector<uint32_t> indices;
  indices.reserve(aabbs.size());
  for (uint32_t i = 0; i < aabbs.size(); i++) {
    indices.push_back(i);
  }

  auto update_node_bounds = [&](uint32_t node_idx) {
    BVH_Node &node = nodes[node_idx];
    node.aabb.min = Vec3<float>(1e30f, 1e30f, 1e30f);
    node.aabb.max = Vec3<float>(-1e30f, -1e30f, -1e30f);
    for (uint32_t first = node.left_first, i = 0; i < node.prim_count; i++) {
      const AABB<float> &aabb = aabbs[indices[first + i]];
      node.aabb = node.aabb.join(aabb);
    }
  };
  std::function<void(uint32_t)> subdivide = [&](uint32_t node_idx) {
    // terminate recursion
    BVH_Node &node = nodes[node_idx];
    if (node.prim_count <= 2)
      return;
    // determine split axis and position
    Vec3<float> extent = node.aabb.max - node.aabb.min;
    int axis = 0;
    if (extent.y > extent.x)
      axis = 1;
    if (extent.z > extent[axis])
      axis = 2;
    float split_pos = node.aabb.min[axis] + extent[axis] * 0.5f;
    // in-place partition
    int i = node.left_first;
    int j = i + node.prim_count - 1;
    while (i <= j) {
      if (aabbs[indices[i]].calc_axis_center(axis) < split_pos)
        i++;
      else
        std::swap(indices[i], indices[j--]);
    }
    // abort split if one of the sides is empty
    int left_count = i - node.left_first;
    if (left_count == 0 || left_count == node.prim_count)
      return;
    // create child nodes
    int left_child_idx = nodes_used++;
    int right_child_idx = nodes_used++;
    nodes[left_child_idx].left_first = node.left_first;
    nodes[left_child_idx].prim_count = left_count;
    nodes[right_child_idx].left_first = i;
    nodes[right_child_idx].prim_count = node.prim_count - left_count;
    node.left_first = left_child_idx;
    node.prim_count = 0;
    update_node_bounds(left_child_idx);
    update_node_bounds(right_child_idx);
    // recurse
    subdivide(left_child_idx);
    subdivide(right_child_idx);
  };

  BVH_Node &root = nodes[root_node_idx];
  root.left_first = 0;
  root.prim_count = (uint32_t)aabbs.size();
  update_node_bounds(root_node_idx);
  // subdivide recursively
  subdivide(root_node_idx);

  return BVH{nodes, indices};
}