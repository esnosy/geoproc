#include <iostream>
#include <fstream>
#include <string>
#include <cstddef>
#include <vector>

template <typename T, size_t N>
struct Vec
{
    T arr[N];
};

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
        for (int j = 0; j < num_tris; j++)
        {
            ifs.seekg(12, ifs.cur);
            for (int i = 0; i < 3; i++)
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

    return 0;
}