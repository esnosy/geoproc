#include <iostream>
#include <fstream>
#include <string>
#include <cstddef>
#include <vector>
#include <functional>
#include <cmath>
#include <random>

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

Vec3f vec3f_cross_product(const Vec3f &a, const Vec3f &b)
{
    float x = a[1] * b[2] - a[2] * b[1];
    float y = a[2] * b[0] - a[0] * b[2];
    float z = a[0] * b[1] - a[1] * b[0];
    return {x, y, z};
}

Vec2f uniform_sample_triangle(const Vec2f &u)
{
    float su0 = std::sqrt(u[0]);
    return {1 - su0, u[1] * su0};
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Expected arguments: /path/to/input.stl num_samples /path/to/output.ply" << std::endl;
        return 1;
    }

    const char *input_filepath = argv[1];
    const size_t num_samples = std::stoull(argv[2]);
    const char *output_filepath = argv[3];

    std::ifstream ifs(input_filepath, std::ifstream::binary);
    ifs.seekg(80, ifs.beg);
    uint32_t num_tris;
    ifs.read(reinterpret_cast<char *>(&num_tris), 4);

    ifs.seekg(0, ifs.end);
    size_t file_size = ifs.tellg();
    size_t expected_file_size = size_t(num_tris) * 50 + 84;

    std::vector<Vec<float, 3>> vertices;
    float x, y, z;

    if (expected_file_size == file_size)
    {
        std::cout << "Binary" << std::endl;
        ifs.seekg(84, ifs.beg);
        for (size_t j = 0; j < num_tris; j++)
        {
            ifs.seekg(12, ifs.cur);
            for (size_t i = 0; i < 3; i++)
            {
                ifs.read(reinterpret_cast<char *>(&x), 4);
                ifs.read(reinterpret_cast<char *>(&y), 4);
                ifs.read(reinterpret_cast<char *>(&z), 4);
                vertices.push_back({x, y, z});
            }
            ifs.seekg(2, ifs.cur);
        }
    }
    else
    {
        std::cout << "ASCII" << std::endl;
        ifs.seekg(0, ifs.beg);
        std::string token;
        while (ifs >> token)
        {
            if (token == "vertex")
            {
                ifs >> x >> y >> z;
                vertices.push_back({x, y, z});
            }
        }
    }
    std::cout << vertices.size() << std::endl;

    std::vector<float> triangle_areas;
    triangle_areas.reserve(vertices.size() / 3);
    size_t i = 0;
    while (i < vertices.size())
    {
        const auto a = vertices[i];
        const auto b = vertices[i + 1];
        const auto c = vertices[i + 2];
        const auto ca = a - c;
        const auto cb = b - c;
        const auto area = 0.5f * vec3f_cross_product(ca, cb).magnitude();
        triangle_areas.push_back(area);
        i += 3;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<size_t> dd(triangle_areas.begin(), triangle_areas.end());
    std::uniform_real_distribution<float> ud(0.0f, 1.0f);

    std::vector<Vec3f> output_points;
    output_points.reserve(num_samples);

    for (size_t i = 0; i < num_samples; i++)
    {
        size_t triangle_index = dd(gen);
        const auto a = vertices[triangle_index * 3];
        const auto b = vertices[triangle_index * 3 + 1];
        const auto c = vertices[triangle_index * 3 + 2];
        const auto ca = a - c;
        const auto cb = b - c;

        float u = ud(gen);
        float v = ud(gen);
        const auto st = uniform_sample_triangle({u, v});
        const auto p = ca * st[0] + cb * st[1] + c;
        output_points.push_back(p);
    }

    std::ofstream ofs(output_filepath, std::ofstream::binary);
    ofs << "ply\n";
    ofs << "format binary_little_endian 1.0\n";
    ofs << "element vertex " << output_points.size() << "\n";
    ofs << "property float x\n";
    ofs << "property float y\n";
    ofs << "property float z\n";
    ofs << "end_header\n";
    ofs.write(reinterpret_cast<char *>(&output_points[0]), 12 * output_points.size());

    return 0;
}