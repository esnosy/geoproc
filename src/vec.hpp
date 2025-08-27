#pragma once

#include <cmath>
#include <functional>

// TODO: split to cpp and hpp

template <typename T, size_t N>
struct Vec
{
    T arr[N];

    T &operator[](size_t i)
    {
        return arr[i];
    }

    Vec binary_op(const Vec &other, std::function<T(T, T)> op) const
    {
        Vec result;
        for (size_t i = 0; i < N; i++)
        {
            result[i] = op(arr[i], other[i]);
        }
        return result;
    }

    Vec binary_op_scalar(const T &other, std::function<T(T, T)> op) const
    {
        Vec result;
        for (size_t i = 0; i < N; i++)
        {
            result[i] = op(arr[i], other);
        }
        return result;
    }

    Vec operator+(const Vec &other) const
    {
        return binary_op(other, [](T a, T b)
                         { return a + b; });
    }

    Vec operator-(const Vec &other) const
    {
        return binary_op(other, [](T a, T b)
                         { return a - b; });
    }

    Vec operator*(const T &other) const
    {
        return binary_op_scalar(other, [](T a, T b)
                                { return a * b; });
    }

    T dot(const Vec &other) const
    {
        T result = arr[0] * other[0];
        for (size_t i = 1; i < N; i++)
        {
            result += arr[i] * other[i];
        }
        return result;
    }

    T magnitude() const
    {
        return std::sqrt(dot(*this));
    }

    const T &operator[](size_t i) const
    {
        return arr[i];
    }
};

using Vec3f = Vec<float, 3>;
using Vec2f = Vec<float, 2>;