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

void fill_circle(Image &image, size_t center_row, size_t center_col, size_t radius, uint8_t red, uint8_t green, uint8_t blue)
{
    const auto width = image.get_width();
    const auto height = image.get_height();
    for (size_t row = center_row; row < radius + center_row; row++)
    {
        for (size_t col = center_col; col < radius + center_col; col++)
        {
            auto ir = row - center_row;
            auto jr = col - center_col;
            auto rr = radius;
            if ((ir * ir + jr * jr) <= (rr * rr))
            {
                if ((row < height) && (col < width))
                    image.set_pixel(row, col, red, green, blue);

                auto ir1 = center_row - ir;
                auto jr1 = center_col - jr;

                if ((ir1 < height) && (jr1 < width))
                    image.set_pixel(ir1, jr1, red, green, blue);

                auto ir2 = center_row - ir;
                auto jr2 = center_col + jr;

                if ((ir2 < height) && (jr2 < width))
                    image.set_pixel(ir2, jr2, red, green, blue);

                auto ir3 = center_row + ir;
                auto jr3 = center_col - jr;

                if ((ir3 < height) && (jr3 < width))
                    image.set_pixel(ir3, jr3, red, green, blue);
            }
        }
    }
}

int main(int argc, char **argv)
{
    Image image(1920, 1080);
    image.clear(255, 255, 255);
    fill_circle(image, 1080 / 2, 1920 / 2, 256, 255, 0, 0);
    image.write_ppm("image.ppm");
    return 0;
}