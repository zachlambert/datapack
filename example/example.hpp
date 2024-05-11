#pragma once

#include <datapack/reader.hpp>
#include <datapack/writer.hpp>
#include <unordered_map>

struct Circle {
    double radius;
};

void read(datapack::Reader& reader, Circle& value);
void write(datapack::Writer& writer, const Circle& value);
static_assert(datapack::readable<Circle> && datapack::writeable<Circle>);

struct Rect {
    double width;
    double height;
};

void read(datapack::Reader& reader, Rect& value);
void write(datapack::Writer& writer, const Rect& value);
static_assert(datapack::readable<Rect> && datapack::writeable<Rect>);

enum class Physics {
    Dynamic,
    Kinematic,
    Static
};

template <>
struct datapack::enum_details<Physics> {
    static std::vector<const char*> labels;
    static const char* to_label(const Physics& value);
    static Physics from_label(const char* label);
};
static_assert(datapack::annotated_enum<Physics>);

using Shape = std::variant<Circle, Rect>;

template <>
struct datapack::variant_details<Shape> {
    static std::vector<const char*> labels;
    static const char* to_label(const Shape& value);
    static Shape from_label(const char* label);
};
static_assert(datapack::annotated_variant<Shape>);

struct Pose {
    double x;
    double y;
    double angle;
};

void read(datapack::Reader& reader, Pose& value);
void write(datapack::Writer& writer, const Pose& value);
static_assert(datapack::readable<Pose> && datapack::writeable<Pose>);

struct Item {
    std::size_t count;
    std::string name;
};

void read(datapack::Reader& reader, Item& value);
void write(datapack::Writer& writer, const Item& value);
static_assert(datapack::readable<Pose> && datapack::writeable<Pose>);

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

void read(datapack::Reader& reader, Sprite& value);
void write(datapack::Writer& writer, const Sprite& value);
static_assert(datapack::readable<Sprite> && datapack::writeable<Sprite>);

struct Entity : public datapack::Readable, public datapack::Writeable {
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

    void read(datapack::Reader& reader) override;
    void write(datapack::Writer& writer) const override;

    static Entity example();
};
static_assert(datapack::readable<Entity> && datapack::writeable<Entity>);
