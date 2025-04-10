#include <cassert>
#include <cmath>
#include <cstring>

#include <array>
#include <limits>
#include <memory>
#include <png.h>
#include <string>
#include <vector>

float constexpr eps = std::numeric_limits<float>::epsilon() * 250.0;

//
// Types and operators
//
typedef std::array<float, 3> vec3;

template <size_t N> constexpr float dot(std::array<float, N> const& lhs, std::array<float, N> const& rhs)
{
    float result{0.0};
    for (size_t i = 0; i < N; i++) {
        result += lhs[i] * rhs[i];
    }
    return result;
}

template <size_t N> constexpr std::array<float, N> normalize(std::array<float, N> const& v)
{
    float length = std::sqrt(dot(v, v));
    std::array<float, N> result{};
    if (length == 0.0) {
        return result;
    }
    for (size_t i = 0; i < N; i++) {
        result[i] = v[i] / length;
    }
    return result;
};

constexpr vec3 cross(vec3 const& lhs, vec3 const& rhs)
{
    vec3 result{};
    result[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
    result[1] = lhs[2] * rhs[0] - lhs[0] * rhs[2];
    result[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
    return result;
}

template <size_t N> constexpr std::array<float, N> operator*(float lhs, std::array<float, N> const& rhs)
{
    std::array<float, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = lhs * rhs[i];
    }
    return result;
}

template <size_t N> constexpr std::array<float, N> operator*(std::array<float, N> const& lhs, float rhs)
{
    std::array<float, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = lhs[i] * rhs;
    }
    return result;
}

template <size_t N>
constexpr std::array<float, N> operator-(std::array<float, N> const& lhs, std::array<float, N> const& rhs)
{
    std::array<float, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = lhs[i] - rhs[i];
    }
    return result;
}

template <size_t N>
constexpr std::array<float, N> operator+(std::array<float, N> const& lhs, std::array<float, N> const& rhs)
{
    std::array<float, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = lhs[i] + rhs[i];
    }
    return result;
}

template <size_t N>
constexpr std::array<float, N> operator*(std::array<float, N> const& lhs, std::array<float, N> const& rhs)
{
    std::array<float, N> result{};
    for (size_t i = 0; i < N; i++) {
        result[i] = lhs[i] * rhs[i];
    }
    return result;
}

template <size_t N>
constexpr std::array<float, N>& operator+=(std::array<float, N>& lhs, std::array<float, N> const& rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

std::array<uint8_t, 4> to_uints(std::array<float, 3> const& data)
{
    std::array<uint8_t, 4> result{};
    for (size_t i = 0; i < 3; i++) {
        result[i] = std::max(0, std::min(static_cast<int>(std::round(data[i] * 255.0f)), 255));
    }
    result[3] = 255;
    return result;
}

//
// Image
//
enum ImageChannelType { RGB = 3, RGBA = 4 };

template <size_t Width, size_t Height, ImageChannelType Channels> class Image
{
    // clang-format off
    std::unique_ptr<
        std::array<
            std::array<
                std::array<uint8_t, Channels>, 
                Width
            >, 
            Height
        >
    > data;
    // clang-format on
public:
    Image(Image const&) = delete;
    Image(Image&&) = delete;
    Image& operator=(Image const&) = delete;
    Image& operator=(Image&&) = delete;
    Image() : data{std::make_unique<std::array<std::array<std::array<uint8_t, Channels>, Width>, Height>>()} {}

    void save(std::string const& filename)
    {
        FILE* file_ptr = fopen(filename.c_str(), "wb");
        if (file_ptr == nullptr) {
            throw 1;
        }
        png_structp write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!write_ptr) {
            fclose(file_ptr);
            throw 2;
        }
        png_init_io(write_ptr, file_ptr);

        png_infop info_ptr = png_create_info_struct(write_ptr);
        if (!info_ptr) {
            png_destroy_write_struct(&write_ptr, (png_infopp)NULL);
            fclose(file_ptr);
            throw 3;
        }
        png_set_IHDR(
            write_ptr,
            info_ptr,
            Width,
            Height,
            sizeof(uint8_t) * 8,
            (Channels == ImageChannelType::RGBA) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );
        png_write_info(write_ptr, info_ptr);

        png_bytepp row_pointers = (png_bytepp)png_malloc(write_ptr, sizeof(png_bytepp) * Height);
        for (size_t i = 0; i < Height; i++) {
            row_pointers[i] = (png_bytep)data->at(i).data();
        }
        png_write_image(write_ptr, row_pointers);

        free(row_pointers);
        png_write_end(write_ptr, info_ptr);
        png_destroy_write_struct(&write_ptr, &info_ptr);
        fclose(file_ptr);
    }

    void set(size_t row, size_t col, std::array<uint8_t, Channels> const& val)
    {
        std::memcpy((*data.get())[row][col].data(), val.data(), sizeof(uint8_t) * Channels);
    }
};

struct Ray {
    vec3 origin;
    vec3 direction;
    float t;

    explicit Ray(vec3 const& origin, vec3 const& direction)
        : origin{origin}, direction{direction}, t{std::numeric_limits<float>::max()}
    {
    }
    Ray() : origin{}, direction{}, t{std::numeric_limits<float>::max()} {}
    Ray(const Ray&) = default;
    Ray(Ray&&) = default;
    Ray& operator=(const Ray&) = default;
    Ray& operator=(Ray&&) = default;

    vec3 hit_position() { return origin + t * direction; }
};

struct Material {
    vec3 color;
    float ambient;
    float diffuse;
    float reflect;

    Material() : color{}, ambient{}, diffuse{}, reflect{} {};
    Material(const Material&) = default;
    Material(Material&&) = default;
    Material& operator=(const Material&) = default;
    Material& operator=(Material&&) = default;
};

struct Object {
    virtual bool hit(Ray& ray) const = 0;
    virtual vec3 normal(vec3 const& hit_position) const = 0;
    virtual Material const& material() const = 0;
};

class Sphere : public Object
{
    vec3 position;
    float radius;

    Material mat;

public:
    Sphere(vec3 const& position, float radius, Material const& mat) : position(position), radius(radius), mat(mat) {}
    Sphere(const Sphere&) = delete;
    Sphere(Sphere&&) = default;
    Sphere& operator=(const Sphere&) = delete;
    Sphere& operator=(Sphere&&) = default;

    bool hit(Ray& ray) const
    {
        vec3 h = position - ray.origin;
        float m = dot(h, ray.direction);
        float g = m * m - dot(h, h) + radius * radius;
        if (g < 0) {
            return false;
        }
        float t0 = m - sqrt(g);
        float t1 = m + sqrt(g);
        if (t0 > eps && t0 < ray.t) {
            ray.t = t0;
            return true;
        } else if (t1 > eps && t1 < ray.t) {
            ray.t = t1;
            return true;
        }
        return false;
    }
    vec3 normal(vec3 const& hit_position) const { return normalize(hit_position - position); };

    Material const& material() const { return mat; }
};

class Triangle : public Object
{
    std::array<vec3, 3> positions;

    Material mat;

public:
    Triangle(std::array<vec3, 3> const& positions, Material const& mat) : positions(positions), mat(mat) {}
    Triangle(const Triangle&) = delete;
    Triangle(Triangle&&) = default;
    Triangle& operator=(const Triangle&) = delete;
    Triangle& operator=(Triangle&&) = default;

    bool hit(Ray& ray) const
    {
        vec3 const e1 = positions[1] - positions[0];
        vec3 const e2 = positions[2] - positions[0];
        vec3 const n = cross(e1, e2);
        float const D = -dot(positions[0], n);
        float const denominator = dot(n, ray.direction);
        if (!std::isnormal(denominator)) {
            return false;
        }
        float const time = -(D + dot(n, ray.origin)) / denominator;
        if (std::signbit(time)) {
            return false;
        }
        if (time > eps && time < ray.t) {
            ray.t = time;
        } else {
            return false;
        }
        vec3 const solution_position = ray.origin + (time * ray.direction);
        vec3 const ep = solution_position - positions[0];
        float const d11 = dot(e1, e1);
        float const d12 = dot(e1, e2);
        float const d22 = dot(e2, e2);
        float const d1p = dot(e1, ep);
        float const d2p = dot(e2, ep);
        float const det = d11 * d22 - d12 * d12;
        if (!std::isnormal(det)) {
            return false;
        }
        float const beta = (d22 * d1p - d12 * d2p) / det;
        float const gamma = (d11 * d2p - d12 * d1p) / det;
        // float const alpha = 1 - beta - gamma;
        if (beta < 0.0 || beta > 1.0 || gamma < 0.0 || gamma > 1.0 || beta + gamma > 1.0 || beta + gamma < 0.0) {
            return false;
        }
        return true;
    }

    vec3 normal(vec3 const& hit_position) const
    {
        return normalize(cross((hit_position - positions[0]), (positions[2] - positions[0])));
    };

    Material const& material() const { return mat; }
};

//
// Point Light
//
struct Light {
    vec3 position;
    vec3 color;

    explicit Light(vec3 const& position, vec3 const& color) : position(position), color(color) {};
    Light(const Light&) = delete;
    Light(Light&&) = default;
    Light& operator=(const Light&) = delete;
    Light& operator=(Light&&) = default;
};

//
// Scene
//
class Scene
{
    std::vector<Light> lights;

    std::vector<std::unique_ptr<Sphere>> sphere_storage;
    std::vector<std::unique_ptr<Triangle>> triangle_storage;
    std::vector<Object*> objects;

public:
    Scene() : lights{}, sphere_storage{}, triangle_storage{}, objects{} {};
    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

    void push_object(Sphere&& sphere)
    {
        sphere_storage.push_back(std::make_unique<Sphere>(std::move(sphere)));
        objects.push_back(static_cast<Object*>(sphere_storage[sphere_storage.size() - 1].get()));
    }

    void push_object(Triangle&& triangle)
    {
        triangle_storage.push_back(std::make_unique<Triangle>(std::move(triangle)));
        objects.push_back(static_cast<Object*>(triangle_storage[triangle_storage.size() - 1].get()));
    }

    void push_light(Light&& light) { lights.push_back(std::move(light)); }

    std::vector<Object*> const& get_objects() const { return objects; };
    std::vector<Light> const& get_lights() const { return lights; };
};

template <size_t Width, size_t Height, size_t MaxDepth> vec3 ray_trace(Scene const& scene, int i, int j)
{
    vec3 color{};
    float intensity{1.0};
    Ray ray{
        {static_cast<float>(i - static_cast<int>(Width / 2)),
         static_cast<float>(j - static_cast<int>(Height / 2)),
         -1000.0},
        {0, 0, 1}
    };

    for (size_t depth = 0; depth < MaxDepth; depth++) {
        Object const* hit_object = nullptr;
        for (auto const object : scene.get_objects()) {
            if (object->hit(ray)) {
                hit_object = object;
            }
        }
        if (hit_object == nullptr) {
            return color;
        }

        vec3 hit_position = ray.hit_position();
        vec3 const hit_normal = hit_object->normal(hit_position);
        hit_position += hit_normal * eps;

        Material const& hit_material = hit_object->material();

        // ambient
        color += intensity * hit_material.ambient * hit_material.color;

        // diffuse
        for (auto const& light : scene.get_lights()) {
            vec3 light_direction = normalize(light.position - hit_position);
            float diffuse = dot(hit_normal, light_direction);
            if (diffuse <= 0.0) {
                continue;
            }

            Ray ray_to_light{hit_position, light_direction};
            bool in_shadow{false};
            for (auto const object : scene.get_objects()) {
                if (object->hit(ray_to_light)) {
                    in_shadow = true;
                    break;
                }
            }
            if (!in_shadow) {
                color += intensity * hit_material.diffuse * diffuse * light.color * hit_material.color;
            }
        }
        intensity *= hit_material.reflect;
        if (intensity < 0.01) {
            return color;
        }
        ray = Ray(hit_position, normalize(ray.direction - 2.0 * dot(ray.direction, hit_normal) * hit_normal));
    }
    return color;
}

//
// Main
//
int main()
{
    constexpr int Width = 512;
    constexpr int Height = 512;
    constexpr int Depth = 10;

    Material mirror;
    mirror.color = {0.9, 1.0, 0.9};
    mirror.ambient = 0.01;
    mirror.diffuse = 0.99;
    mirror.reflect = 0.99;
    Material matte;
    matte.color = {1.0, 0.8, 0.6};
    matte.ambient = 0.3;
    matte.diffuse = 0.7;
    matte.reflect = 0.2;

    Scene scene{};
    scene.push_light(Light({-500, 0, 100}, {1, 0, 0}));
    scene.push_light(Light({+500, 0, 100}, {0, 1, 0}));
    scene.push_light(Light({0, +500, -100}, {0, 0, 1}));
    scene.push_light(Light({0, -500, -100}, {0, 1, 1}));
    scene.push_light(Light({0, 0, 100}, {1, 1, 0}));
    scene.push_object(Sphere({-87, -50, 0}, 100, mirror));
    scene.push_object(Sphere({+87, -50, 0}, 100, mirror));
    scene.push_object(Sphere({0, 100, 0}, 100, matte));
    scene.push_object(Triangle({{{-1000, -1000, 0}, {1000, -1000, 0}, {1000, 1000, 0}}}, matte));
    Image<Width, Height, ImageChannelType::RGBA> img{};
    for (size_t i = 0; i < Width; i++) {
        for (size_t j = 0; j < Height; j++) {
            img.set(i, j, to_uints(ray_trace<Width, Height, Depth>(scene, i, j)));
        }
    }
    img.save("example.png");
}
