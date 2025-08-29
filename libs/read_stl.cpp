#include <fstream>
#include <iostream>
#include <vector>

#include "vec.hpp"
#include "fast_float.h"

std::vector<Vec3f> read_stl(const char *input_filepath)
{
    std::ifstream ifs(input_filepath, std::ifstream::binary);
    ifs.seekg(80, ifs.beg);
    uint32_t num_tris;
    ifs.read(reinterpret_cast<char *>(&num_tris), 4);

    ifs.seekg(0, ifs.end);
    size_t file_size = ifs.tellg();
    size_t expected_file_size = size_t(num_tris) * 50 + 84;

    std::vector<Vec3f> vertices;
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
                ifs >> token;
                fast_float::from_chars(token.data(), token.data() + token.size(), x);
                ifs >> token;
                fast_float::from_chars(token.data(), token.data() + token.size(), y);
                ifs >> token;
                fast_float::from_chars(token.data(), token.data() + token.size(), z);

                vertices.push_back({x, y, z});
            }
        }
    }

    return vertices;
}