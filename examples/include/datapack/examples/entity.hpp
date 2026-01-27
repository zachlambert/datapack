#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include <datapack/datapack.hpp>
#include <datapack/labelled_enum.hpp>
#include <datapack/labelled_variant.hpp>
#include <datapack/std/string.hpp>

struct Circle {
  double radius;
};

struct Rect {
  double width;
  double height;
};

enum class Physics { Dynamic, Kinematic, Static };

using Shape = std::variant<Circle, Rect>;

struct Pose {
  double x;
  double y;
  double angle;
};

struct Item {
  std::size_t count;
  std::string name;
  DATAPACK_CLASS_INLINE(count, name)
};

struct Sprite {
  struct Pixel {
    double r;
    double g;
    double b;
    DATAPACK_CLASS_INLINE(r, g, b)
  };
  std::size_t width;
  std::size_t height;
  std::vector<Pixel> data;
  DATAPACK_CLASS_DECL()
};

struct Entity {
  int index; // Primitives
  std::string name;
  bool enabled;
  Pose pose;                         // Object
  Physics physics;                   // Enum
  std::optional<Shape> hitbox;       // Optional + Variant
  Sprite sprite;                     // Binary
  std::vector<Item> items;           // List
  std::array<int, 3> assigned_items; // Tuple

  static Entity example();
  DATAPACK_CLASS_DECL()
};

bool operator==(const Entity& a, const Entity& b);

namespace datapack {

DATAPACK_INLINE(Circle, radius)
DATAPACK_INLINE(Rect, width, height)
DATAPACK_INLINE(Pose, x, y, angle)

DATAPACK_LABELLED_ENUM(Physics, 3);
DATAPACK_LABELLED_VARIANT(Shape, 3);

}
