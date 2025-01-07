#pragma once

#include <array>
#include <memory>

#include <png.h>

#include "arrayalgebra.h"

constexpr aa::u8vec4 BackgroundColor{0, 0, 0, 255};

inline aa::u8vec4 ftou8(aa::vec4 v)
{
    for (size_t i = 0; i < v.size(); i++) {
        if (v[i] > 1.0) {
            v[i] = 1.0;
        } else if (v[i] < 0.0) {
            v[i] = 0.0;
        }
    }
    constexpr float int_conversion = 255.999;
    uint8_t r = static_cast<uint8_t>(v[0] * int_conversion);
    uint8_t g = static_cast<uint8_t>(v[1] * int_conversion);
    uint8_t b = static_cast<uint8_t>(v[2] * int_conversion);
    uint8_t a = static_cast<uint8_t>(v[3] * int_conversion);
    return {r, g, b, a};
};

template <size_t Width, size_t Height> class Image
{
    std::unique_ptr<std::array<std::array<aa::u8vec4, Width>, Height>> data;

public:
    Image()
    {
        data =
            std::make_unique<std::array<std::array<aa::u8vec4, Width>, Height>>(
            );
        for (size_t hi = 0; hi < Height; hi++) {
            for (size_t wi = 0; wi < Height; wi++) {
                this->ref(hi, wi) = BackgroundColor;
            }
        }
    }

    aa::u8vec4& ref(size_t hi, size_t wi) { return (*data.get())[hi][wi]; }

    size_t width() { return Width; }
    size_t height() { return Height; }

    int save(std::string const& filename) const
    {
        // for now these are just the default
        int constexpr bit_depth = 8;
        int constexpr color_channels = 4;

        FILE* outfile = fopen(filename.c_str(), "wb");

        png_structp png_ptr =
            png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            std::cerr << "png_create_write_struct()\n";
            return 1;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
            std::cerr << "png_create_info_struct()\n";
            return 2;
        }

        png_init_io(png_ptr, outfile);
        png_set_IHDR(
            png_ptr,
            info_ptr,
            Width,
            Height,
            bit_depth,
            PNG_COLOR_TYPE_RGB_ALPHA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );

        png_bytepp row_pointers =
            (png_bytepp)png_malloc(png_ptr, sizeof(png_bytepp) * Height);
        for (size_t i = 0; i < Height; i++) {
            row_pointers[i] =
                (png_bytep)png_malloc(png_ptr, Width * color_channels);
        }
        for (size_t hi = 0; hi < Height; hi++) {
            for (size_t wi = 0; wi < Width; wi++) {
                row_pointers[hi][(wi * color_channels) + 0] =
                    (*data.get())[hi][wi][0];
                row_pointers[hi][(wi * color_channels) + 1] =
                    (*data.get())[hi][wi][1];
                row_pointers[hi][(wi * color_channels) + 2] =
                    (*data.get())[hi][wi][2];
                row_pointers[hi][(wi * color_channels) + 3] =
                    (*data.get())[hi][wi][3];
            }
        }

        png_write_info(png_ptr, info_ptr);
        png_write_image(png_ptr, row_pointers);
        png_write_end(png_ptr, info_ptr);

        for (size_t y = 0; y < Height; y++) {
            free(row_pointers[y]);
        }
        free(row_pointers);

        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(outfile);
        return 0;
    }
};
