#include "vec.hpp"

template <typename T, size_t N> struct AABB {
  Vec<T, N> max, min;
  Vec<T, N> calc_dims() {
    return max - min;
  }
};