#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <stack>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "../libs/aabb.hpp"
#include "../libs/stl_io.hpp"

#ifndef _WIN32
#define _aligned_malloc(size, alignment) std::aligned_alloc(alignment, size)
#define _aligned_free(ptr) std::free(ptr)
#endif

template <typename T> struct Indexed_Mesh {
  std::vector<Vec3<T>> vertices;
  std::vector<std::array<uint32_t, 3>> tris;

  static Indexed_Mesh from_stl_tris(const std::vector<Triangle<T>> &tris) {
    std::unordered_map<Vec3<T>, uint32_t> vertex_to_index;
    vertex_to_index.reserve(tris.size() * 3);
    Indexed_Mesh<T> mesh;
    mesh.tris.reserve(tris.size());
    mesh.vertices.reserve(tris.size() * 3);
    for (const auto &t : tris) {
      std::array<uint32_t, 3> indexed_tri;

      for (int i = 0; i < 3; i++) {
        auto v_it = vertex_to_index.find(t[i]);
        if (v_it == vertex_to_index.end()) {
          mesh.vertices.push_back(t[i]);
          vertex_to_index[t[i]] = vertex_to_index.size();
          indexed_tri[i] = vertex_to_index.size() - 1;
        } else {
          indexed_tri[i] = v_it->second;
        }
      }
      mesh.tris.push_back(indexed_tri);
    }
    return mesh;
  }

  std::vector<Vec3<T>> calc_vertex_normals() const {
    std::vector<Vec3<T>> vertex_normals;
    vertex_normals.resize(vertices.size(), Vec3<T>(0, 0, 0));
    for (size_t i = 0; i < tris.size(); i++) {
      const auto &indexed_tri = tris[i];
      const auto &tri =
          Triangle<T>{vertices[indexed_tri[0]], vertices[indexed_tri[1]],
                      vertices[indexed_tri[2]]};
      const auto &n = tri.calc_normal_unnormalized();
      for (int j = 0; j < 3; j++) {
        vertex_normals[indexed_tri[j]] += n;
      }
    }
    for (auto &n : vertex_normals) {
      n = n.normalized();
    }
    return vertex_normals;
  }
};

struct BVH_Node {
  AABB<float> aabb;
  uint32_t left_first, prim_count;
  bool is_leaf() { return prim_count > 0; }
};

template <typename T> struct Ray_Triangle_Intersection {
  T t, u, v;
  bool hit = false;
};

template <typename T> struct Ray {
  Vec3<T> origin;
  Vec3<T> direction;
  Ray_Triangle_Intersection<T> intersection;
  size_t tri_idx;
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
  BVH_Node *nodes;
  std::vector<uint32_t> indices;

  void free() { _aligned_free(nodes); }

  template <typename T>
  void intersect_tris(Ray<T> &ray, const std::vector<Triangle<T>> &tris) {
    std::stack<uint32_t> stack;
    stack.push(0);
    while (!stack.empty()) {
      uint32_t node_idx = stack.top();
      stack.pop();
      BVH_Node &node = nodes[node_idx];
      AABB<T> node_aabb{node.aabb.min.as<T>(), node.aabb.max.as<T>()};
      if (!intersect_ray_aabb(ray, node_aabb)) {
        continue;
      }
      if (node.is_leaf()) {
        for (uint32_t i = 0; i < node.prim_count; i++) {
          auto tri_idx = indices[node.left_first + i];
          auto result = intersect_ray_triangle(ray, tris[tri_idx]);
          if (result.hit) {
            if (ray.intersection.hit) {
              if (result.t < ray.intersection.t) {
                ray.intersection = result;
                ray.tri_idx = tri_idx;
              }
            } else {
              ray.intersection = result;
              ray.tri_idx = tri_idx;
            }
          }
        }
      } else {
        stack.push(node.left_first);
        stack.push(node.left_first + 1);
      }
    }
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
    node.aabb.min = Vec3<float>(1e30f);
    node.aabb.max = Vec3<float>(-1e30f);
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

  auto mesh = Indexed_Mesh<double>::from_stl_tris(tris);
  auto vertex_normals = mesh.calc_vertex_normals();

  std::vector<Triangle<float>> tris_float;
  tris_float.reserve(tris.size());
  for (const auto &t : tris) {
    tris_float.push_back({t.a.as<float>(), t.b.as<float>(), t.c.as<float>()});
  }

  std::vector<AABB<float>> aabbs;
  aabbs.reserve(tris_float.size());
  for (const auto &t : tris_float) {
    const auto &aabb = t.calc_aabb();
    aabbs.push_back(aabb);
  }

  t0 = std::chrono::high_resolution_clock::now();
  auto bvh = build_bvh(aabbs);
  t1 = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration<double, std::milli>(t1 - t0);
  std::cout << "Built BVH in " << duration.count() << " ms" << std::endl;

  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 1920 * 4;

  // Calculate the image height, and ensure that it's at least 1.
  int image_height = int(image_width / aspect_ratio);
  image_height = (image_height < 1) ? 1 : image_height;

  // Camera

  auto focal_length = 1.0;
  auto viewport_height = 2.0;
  auto viewport_width = viewport_height * (double(image_width) / image_height);
  auto camera_center = Vec3<double>(0, 0, 0);

  // Calculate the vectors across the horizontal and down the vertical viewport
  // edges.
  auto viewport_u = Vec3<double>(viewport_width, 0, 0);
  auto viewport_v = Vec3<double>(0, -viewport_height, 0);

  // Calculate the horizontal and vertical delta vectors from pixel to pixel.
  auto pixel_delta_u = viewport_u / image_width;
  auto pixel_delta_v = viewport_v / image_height;

  // Calculate the location of the upper left pixel.
  auto viewport_upper_left = camera_center - Vec3<double>(0, 0, focal_length) -
                             viewport_u / 2 - viewport_v / 2;
  auto pixel00_loc =
      viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

  std::ofstream ofs("output.ppm", std::ios::binary);
  ofs << "P3\n" << image_width << " " << image_height << "\n255\n";

  t0 = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < image_height; j++) {
    for (int i = 0; i < image_width; i++) {
      auto pixel_center = pixel00_loc + (double(i) * pixel_delta_u) +
                          (double(j) * pixel_delta_v);
      auto ray_direction = pixel_center - camera_center;
      Ray<double> ray(camera_center, ray_direction);
      bvh.intersect_tris(ray, tris);
      if (ray.intersection.hit) {
        const auto &tri = mesh.tris[ray.tri_idx];
        const auto &n1 = vertex_normals[tri[0]];
        const auto &n2 = vertex_normals[tri[1]];
        const auto &n3 = vertex_normals[tri[2]];
        auto normal = (1 - ray.intersection.u - ray.intersection.v) * n1 +
                      ray.intersection.u * n2 + ray.intersection.v * n3;
        uint32_t c = std::clamp(
            std::abs(normal.dot(-ray.direction.normalized())) * 255.0, 0.0,
            255.0);
        ofs << c << ' ' << c << ' ' << c << '\n';
      } else {
        ofs << "0 0 0\n";
      }
    }
  }
  t1 = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration<double, std::milli>(t1 - t0);
  std::cout << "Rendered image in " << duration.count() << " ms" << std::endl;

  uint32_t node_count = 0;
  uint32_t num_leaf_nodes = 0;
  uint32_t max_prim_count = 0;

  std::stack<BVH_Node *> stack;
  stack.push(bvh.nodes);
  while (!stack.empty()) {
    auto node = stack.top();
    stack.pop();
    if (node->is_leaf()) {
      max_prim_count = std::max(max_prim_count, node->prim_count);
      num_leaf_nodes++;
    } else {
      stack.push(bvh.nodes + node->left_first);
      stack.push(bvh.nodes + node->left_first + 1);
    }
    node_count++;
  }

  bvh.free();

  std::cout << "Node count: " << node_count << std::endl;
  std::cout << "Max prim. count: " << max_prim_count << std::endl;
  std::cout << "Num. leaf nodes: " << num_leaf_nodes << std::endl;
}