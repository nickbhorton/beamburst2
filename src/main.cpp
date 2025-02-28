#include "arrayalgebra.h"
#include "image.h"

using namespace aa;

int main()
{
    float constexpr ASPECT_RATIO{16.0 / 9.0};
    float constexpr VIEWPORT_WIDTH{1};
    float constexpr VIEWPORT_HEIGHT{VIEWPORT_WIDTH / ASPECT_RATIO};
    size_t constexpr IMAGE_WIDTH{2000};
    size_t constexpr IMAGE_HEIGHT{
        static_cast<size_t>(IMAGE_WIDTH / ASPECT_RATIO)
    };

    static vec3 constexpr CAMERA_POSITION{0, 0, 0};
    static vec3 constexpr CAMERA_DIRECTION{0, 0, -1};
    static vec3 constexpr CAMERA_RIGHT{1, 0, 0};
    static vec3 constexpr CAMERA_UP{cross(CAMERA_RIGHT, CAMERA_DIRECTION)};
    float constexpr VIEWPORT_DISTANCE_FROM_CAMERA{1};

    auto constexpr get_ray_direction = [&](float u, float v) {
        float const x = (2.0 * (u - 0.5)) * VIEWPORT_WIDTH;
        float const y = (2.0 * (v - 0.5)) * VIEWPORT_HEIGHT;
        return (VIEWPORT_DISTANCE_FROM_CAMERA * CAMERA_DIRECTION +
                x * CAMERA_RIGHT + y * CAMERA_UP) -
               CAMERA_POSITION;
    };

    static vec3 constexpr SPHERE_POSITION{0, 0, -10};
    float constexpr SPHERE_RADIUS{2};

    auto constexpr intersect_sphere = [&](vec3 const& ray_direction) -> float {
        float const a = dot(ray_direction, ray_direction);
        float const b = 2.0 * (dot(CAMERA_POSITION, ray_direction) -
                               dot(ray_direction, SPHERE_POSITION));
        float const c = dot(CAMERA_POSITION, CAMERA_POSITION) +
                        dot(SPHERE_POSITION, SPHERE_POSITION) -
                        2.0 * dot(CAMERA_POSITION, SPHERE_POSITION) -
                        std::pow(SPHERE_RADIUS, 2.0);
        float const discriminant = b * b - 4.0 * a * c;
        if (std::isinf(discriminant) || std::signbit(discriminant) ||
            std::isnan(discriminant)) {
            return -1.0;
        }
        if (discriminant == 0.0) {
            float t = -b / (2.0 * a);
            if (!std::isinf(t) && !std::signbit(t) && !std::isnan(t)) {
                return t;
            } else {
                return -1.0;
            }
        }
        float const t1 = (-b + std::sqrt(discriminant)) / (2.0 * a);
        float const t2 = (-b - std::sqrt(discriminant)) / (2.0 * a);
        bool const valid_positive_t1 =
            !std::isinf(t1) && !std::signbit(t1) && !std::isnan(t1);
        bool const valid_positive_t2 =
            !std::isinf(t2) && !std::signbit(t2) && !std::isnan(t2);
        if (!valid_positive_t1 && !valid_positive_t2) {
            return {};
        } else if (valid_positive_t1 && !valid_positive_t2) {
            return t1;
        } else if (!valid_positive_t1 && valid_positive_t2) {
            return t2;
        } else {
            return std::min(t1, t2);
        }
    };

    Image<IMAGE_WIDTH, IMAGE_HEIGHT> img{};
    for (size_t j = 0; j < IMAGE_HEIGHT; j++) {
        std::clog << "\r" << (IMAGE_HEIGHT - j) << ' ' << std::flush;
        for (size_t i = 0; i < IMAGE_WIDTH; i++) {
            float const u = double(i) / (IMAGE_WIDTH - 1);
            float const v = double(j) / (IMAGE_HEIGHT - 1);
            auto const ray_direction{get_ray_direction(u, v)};
            auto const t{intersect_sphere(ray_direction)};
            if (t > 0.0) {
                auto const intersection_position{
                    CAMERA_POSITION + t * ray_direction
                };
                auto const intersection_normal{
                    normalize(intersection_position - SPHERE_POSITION)
                };
                img.ref(j, i) = ftou8(
                    {0,
                     0,
                     std::abs(dot(intersection_normal, CAMERA_DIRECTION)),
                     1}
                );
            } else {
                img.ref(j, i) = ftou8({u, v, 0, 1});
            }
        }
    }
    std::clog << "\n";
    img.save("test.png");
}
