#pragma once

#include <array>
#include <optional>
#include <unordered_map>

#include <datapack/datapack.hpp>
#include <datapack/reader.hpp>
#include <datapack/writer.hpp>
#include <datapack/labelled_enum.hpp>
#include <datapack/labelled_variant.hpp>

struct Circle {
    double radius;
};

struct Rect {
    double width;
    double height;
};

enum class Physics {
    Dynamic,
    Kinematic,
    Static
};

using Shape = std::variant<Circle, Rect>;

struct Pose {
    double x;
    double y;
    double angle;
};

struct Item {
    std::size_t count;
    std::string name;
};

struct Sprite {
    struct Pixel {
        double r;
        double g;
        double b;
    };
    std::size_t width;
    std::size_t height;
    std::vector<Pixel> data;
};

struct Entity {
    int index;        // Primitives
    std::string name;
    bool enabled;
    Pose pose;        // Object
    Physics physics;  // Enum
    std::optional<Shape> hitbox; // Optional + Variant
    Sprite sprite;               // Binary
    std::vector<Item> items;     // List
    std::array<int, 3> assigned_items; // Tuple
    std::unordered_map<std::string, double> properties;  // Map with key=string
    std::unordered_map<int, bool> flags; // Map with key!=string -> list of tuples

    static Entity example();

    DATAPACK_METHODS(Entity);
};

bool compare(const Entity& a, const Entity& b, double float_threshold = 1e-12);

namespace datapack {

DATAPACK(Circle)
DATAPACK(Rect);
DATAPACK_LABELLED_ENUM(Physics, 3)
DATAPACK_LABELLED_VARIANT(Shape, 2)
DATAPACK(Pose)
DATAPACK(Item)
DATAPACK(Sprite)
DATAPACK(Sprite::Pixel)

} // namespace datapack
