#include "datapack/examples/entity.hpp"

#include <cmath>
#include <cstring>
#include <datapack/std/array.hpp>
#include <datapack/std/optional.hpp>
#include <datapack/std/string.hpp>
#include <datapack/std/unordered_map.hpp>
#include <datapack/std/variant.hpp>
#include <datapack/std/vector.hpp>

namespace datapack {

DATAPACK_LABELLED_VARIANT_DEF(Shape) = {"circle", "rect"};
DATAPACK_LABELLED_ENUM_DEF(Physics) = {"dynamic", "kinematic", "static"};

} // namespace datapack

void Sprite::read(datapack::Reader& reader) {
  reader.object_begin();
  reader.value("width", width);
  reader.value("height", height);
  reader.object_next("data");
  auto bytes = reader.binary();
  data.resize(bytes.size() / sizeof(Pixel));
  std::memcpy(data.data(), bytes.data(), bytes.size());
  reader.object_end();
}

void Sprite::write(datapack::Writer& writer) const {
  writer.object_begin();
  writer.value("width", width);
  writer.value("height", height);
  writer.object_next("data");
  writer.binary({(const std::uint8_t*)data.data(), data.size() * sizeof(Pixel)});
  writer.object_end();
}

DATAPACK_CLASS_DEF(
    Entity,
    index,
    name,
    enabled,
    pose,
    physics,
    hitbox,
    sprite,
    items,
    assigned_items)

Entity Entity::example() {
  Entity result;
  result.index = 5;
  result.name = "player";
  result.enabled = true;
  result.pose = {1.0, 2.0, 3.0};
  result.physics = Physics::Kinematic;
  result.hitbox = Circle{1.0};
  result.sprite = []() {
    Sprite sprite;
    sprite.width = 2;
    sprite.height = 2;
    sprite.data.resize(sprite.height * sprite.width);
    for (int i = 0; i < sprite.height; i++) {
      for (int j = 0; j < sprite.width; j++) {
        auto& pixel = sprite.data[i * sprite.width + j];
        pixel.r = ((double)i + 0.5) / sprite.height;
        pixel.g = ((double)j + 0.5) / sprite.width;
        pixel.b = 0;
      }
    }
    return sprite;
  }();
  result.items = []() {
    std::vector<Item> items;
    items.push_back(Item{5, "hp_potion"});
    items.push_back(Item{1, "sword"});
    items.push_back(Item{1, "map"});
    items.push_back(Item{120, "gold"});
    return items;
  }();
  result.assigned_items = {1, 2, -1};
  return result;
}

bool operator==(const Entity& a, const Entity& b) {
  static constexpr double float_threshold = 1e-12;
  if (a.index != b.index)
    return false;
  if (a.name != b.name)
    return false;
  if (a.enabled != b.enabled)
    return false;

  if (std::abs(a.pose.x - b.pose.x) > float_threshold)
    return false;
  if (std::abs(a.pose.y - b.pose.y) > float_threshold)
    return false;
  if (std::abs(a.pose.angle - b.pose.angle) > float_threshold)
    return false;

  if (a.physics != b.physics)
    return false;

  if (a.hitbox.has_value() != b.hitbox.has_value())
    return false;
  if (auto a_circle = std::get_if<Circle>(&a.hitbox.value())) {
    auto b_circle = std::get_if<Circle>(&b.hitbox.value());
    if (!b_circle)
      return false;
    if (std::abs(a_circle->radius - b_circle->radius) > float_threshold)
      return false;
  } else if (auto a_rect = std::get_if<Rect>(&a.hitbox.value())) {
    auto b_rect = std::get_if<Rect>(&b.hitbox.value());
    if (!b_rect)
      return false;
    if (a_rect->width != b_rect->width)
      return false;
    if (a_rect->height != b_rect->height)
      return false;
    if (std::abs(a_rect->width - b_rect->width) > float_threshold)
      return false;
    if (std::abs(a_rect->height - b_rect->height) > float_threshold)
      return false;
  }

  if (a.sprite.width != b.sprite.width)
    return false;
  if (a.sprite.height != b.sprite.height)
    return false;
  if (a.sprite.data.size() != b.sprite.data.size())
    return false;
  for (std::size_t i = 0; i < a.sprite.data.size(); i++) {
    const auto& a_pixel = a.sprite.data[i];
    const auto& b_pixel = b.sprite.data[i];
    if (std::abs(a_pixel.r - b_pixel.r) > float_threshold)
      return false;
    if (std::abs(a_pixel.g - b_pixel.g) > float_threshold)
      return false;
    if (std::abs(a_pixel.b - b_pixel.b) > float_threshold)
      return false;
  }

  if (a.items.size() != b.items.size())
    return false;
  for (std::size_t i = 0; i < a.items.size(); i++) {
    const auto& a_item = a.items[i];
    const auto& b_item = b.items[i];
    if (a_item.name != b_item.name)
      return false;
    if (a_item.count != b_item.count)
      return false;
  }

  return true;
}
