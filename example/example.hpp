#pragma once

#include <array>
#include <optional>
#include <unordered_map>

#include <datapack/visitor.hpp>
#include <datapack/reader.hpp>
#include <datapack/writer.hpp>
#include <datapack/labelled_enum.hpp>
#include <datapack/labelled_variant.hpp>

struct Circle {
    double radius;
};
DATAPACK_VISITOR_FUNCS_DEF(Circle)

struct Rect {
    double width;
    double height;
};
DATAPACK_VISITOR_FUNCS_DEF(Rect);

enum class Physics {
    Dynamic,
    Kinematic,
    Static
};
DATAPACK_LABELLED_ENUM(Physics)

using Shape = std::variant<Circle, Rect>;
DATAPACK_LABELLED_VARIANT(Shape)

struct Pose {
    double x;
    double y;
    double angle;
};
DATAPACK_VISITOR_FUNCS_DEF(Pose)

struct Item {
    std::size_t count;
    std::string name;
};
DATAPACK_VISITOR_FUNCS_DEF(Item)

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
DATAPACK_VISITOR_FUNCS_DEF(Sprite)
DATAPACK_VISITOR_FUNCS_DEF(Sprite::Pixel)

struct Entity : public datapack::Visitor {
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
    std::unordered_map<int, bool> flags; // Map with key!=string -> list of tpules

    static Entity example();

    DATAPACK_VISITOR_METHODS_DEF(Entity)
};

bool compare(const Entity& a, const Entity& b, double float_threshold = 1e-12);
