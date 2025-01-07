#include "image.h"

int main()
{
    size_t constexpr IMAGE_WIDTH{512};
    size_t constexpr IMAGE_HEIGHT{256};
    Image<IMAGE_WIDTH, IMAGE_HEIGHT> img{};
    for (size_t j = 0; j < IMAGE_HEIGHT; j++) {
        for (size_t i = 0; i < IMAGE_WIDTH; i++) {
            float const u = double(i) / (IMAGE_WIDTH - 1);
            float const v = double(j) / (IMAGE_HEIGHT - 1);
            img.ref(j, i) = ftou8({u, v, 0, 1.0});
        }
    }
    img.save("test.png");
}
