#include <cstdint>
#include <fstream>

class Image
{
    char *pixels = nullptr;
    size_t num_columns;
    size_t num_rows;

public:
    Image(size_t num_columns, size_t num_rows)
        : num_columns(num_columns), num_rows(num_rows)
    {
        this->pixels = new char[num_columns * num_rows * 3];
        for (size_t i = 0; i < (num_columns * num_rows * 3); i++)
        {
            this->pixels[i] = 0;
        }
    }
    size_t get_num_columns() const { return num_columns; }
    size_t get_num_rows() const { return num_rows; }
    void set_pixel(size_t row, size_t col, uint8_t r, uint8_t g, uint8_t b)
    {
        size_t index = (row * num_columns + col) * 3;
        pixels[index] = r;
        pixels[index + 1] = g;
        pixels[index + 2] = b;
    }
    void get_pixel(size_t row, size_t col, uint8_t *r, uint8_t *g,
                   uint8_t *b) const
    {
        size_t index = (row * num_columns + col) * 3;
        *r = pixels[index];
        *g = pixels[index + 1];
        *b = pixels[index + 2];
    }
    void clear(uint8_t r, uint8_t g, uint8_t b)
    {
        for (size_t i = 0; i < (num_columns * num_rows * 3); i += 3)
        {
            pixels[i] = r;
            pixels[i + 1] = g;
            pixels[i + 2] = b;
        }
    }
    void write_ppm(const char *output_filename) const
    {
        std::ofstream output_file(output_filename, std::ofstream::binary);
        output_file << "P6\n";
        output_file << num_columns << " " << num_rows << "\n";
        output_file << "255\n";
        output_file.write(pixels, num_columns * num_rows * 3);
    }
};

void fill_circle(Image &image, size_t center_row, size_t center_col,
                 size_t radius, uint8_t red, uint8_t green, uint8_t blue)
{
    const auto width = image.get_num_columns();
    const auto height = image.get_num_rows();
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

Image downsample(const Image &image, size_t n)
{
    Image image2(image.get_num_columns() / n, image.get_num_rows() / n);

    for (size_t row = 0; row < image2.get_num_rows(); row++)
    {
        for (size_t col = 0; col < image2.get_num_columns(); col++)
        {
            size_t cr = 0;
            size_t cg = 0;
            size_t cb = 0;
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    size_t original_row = row * n + i;
                    size_t original_col = col * n + j;
                    uint8_t r, g, b;
                    image.get_pixel(original_row, original_col, &r, &g, &b);
                    cr += r;
                    cg += g;
                    cb += b;
                }
            }
            cr /= (n * n);
            cg /= (n * n);
            cb /= (n * n);
            image2.set_pixel(row, col, uint8_t(cr), uint8_t(cg), uint8_t(cb));
        }
    }
    return image2;
}

int main(int argc, char **argv)
{
    Image image(4096, 4096);
    image.clear(255, 255, 255);
    fill_circle(image, 4096 / 2, 4096 / 2, 512, 255, 0, 0);
    image.write_ppm("image.ppm");

    auto image2 = downsample(image, 8);
    image2.write_ppm("image2.ppm");
    return 0;
}