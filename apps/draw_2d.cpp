#include <fstream>
#include <cstdint>

class Image
{
    char *pixels = nullptr;
    size_t width;
    size_t height;

public:
    size_t get_width() const
    {
        return width;
    }
    size_t get_height() const
    {
        return height;
    }
    Image(size_t width, size_t height) : width(width), height(height)
    {
        this->pixels = new char[width * height * 3];
        for (size_t i = 0; i < (width * height * 3); i++)
        {
            this->pixels[i] = 0;
        }
    }
    void set_pixel(size_t row, size_t col, uint8_t r, uint8_t g, uint8_t b)
    {
        size_t index = (row * width + col) * 3;
        pixels[index] = r;
        pixels[index + 1] = g;
        pixels[index + 2] = b;
    }
    void clear(uint8_t r, uint8_t g, uint8_t b)
    {
        for (size_t i = 0; i < (width * height * 3); i += 3)
        {
            pixels[i] = r;
            pixels[i + 1] = g;
            pixels[i + 2] = b;
        }
    }
    void write_ppm(const char *output_filename)
    {
        std::ofstream output_file(output_filename, std::ofstream::binary);
        output_file << "P6\n";
        output_file << width << " " << height << "\n";
        output_file << "255\n";
        output_file.write(pixels, width * height * 3);
    }
};

void fill_circle(Image &image, size_t center_x, size_t center_y, size_t radius, uint8_t red, uint8_t green, uint8_t blue)
{
    const auto width = image.get_width();
    const auto height = image.get_height();
    for (size_t i = center_x; i < radius + center_x; i++)
    {
        for (size_t j = center_y; j < radius + center_y; j++)
        {
            auto ir = i - center_x;
            auto jr = j - center_y;
            auto rr = radius;
            if ((ir * ir + jr * jr) <= (rr * rr))
            {
                if ((i < width) && (j < height))
                    image.set_pixel(i, j, radius, green, blue);

                auto ir1 = center_x - ir;
                auto jr1 = center_y - jr;

                if ((ir1 < width) && (jr1 < height))
                    image.set_pixel(ir1, jr1, radius, green, blue);

                auto ir2 = center_x - ir;
                auto jr2 = center_y + jr;

                if ((ir2 < width) && (jr2 < height))
                    image.set_pixel(ir2, jr2, radius, green, blue);

                auto ir3 = center_x + ir;
                auto jr3 = center_y - jr;

                if ((ir3 < width) && (jr3 < height))
                    image.set_pixel(ir3, jr3, radius, green, blue);
            }
        }
    }
}

int main(int argc, char **argv)
{
    Image image(4096, 4096);
    image.clear(255, 0, 0);
    fill_circle(image, 2048, 2048, 1024, 0, 0, 255);
    image.write_ppm("image.ppm");
    return 0;
}