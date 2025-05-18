#include <datapack/examples/entity.hpp>
#include <datapack/json.hpp>
#include <gtest/gtest.h>

static std::vector<std::string> get_lines(const std::string& text) {
  std::vector<std::string> lines;
  std::size_t prev = 0;
  std::size_t pos = std::min(text.find('\n', 0), text.size());
  while (pos != text.size()) {
    lines.push_back(text.substr(prev, pos - prev));
    prev = pos + 1;
    pos = std::min(text.find('\n', prev), text.size());
  }
  return lines;
}

static const std::string entity_json = R"({
    "index": 5,
    "name": "player",
    "enabled": true,
    "pose": {
        "x": 1,
        "y": 2,
        "angle": 3
    },
    "physics": "kinematic",
    "hitbox": {
        "type": "circle",
        "value_circle": {
            "radius": 1
        }
    },
    "sprite": {
        "width": 2,
        "height": 2,
        "data": [
            {
                "r": 0.25,
                "g": 0.25,
                "b": 0
            },
            {
                "r": 0.25,
                "g": 0.75,
                "b": 0
            },
            {
                "r": 0.75,
                "g": 0.25,
                "b": 0
            },
            {
                "r": 0.75,
                "g": 0.75,
                "b": 0
            }
        ]
    },
    "items": [
        {
            "count": 5,
            "name": "hp_potion"
        },
        {
            "count": 1,
            "name": "sword"
        },
        {
            "count": 1,
            "name": "map"
        },
        {
            "count": 120,
            "name": "gold"
        }
    ],
    "assigned_items": [
        1,
        2,
        -1
    ]
})";

TEST(Format, JsonDump) {
  const std::string output = datapack::write_json(Entity::example());

  auto expected_lines = get_lines(entity_json);
  auto output_lines = get_lines(output);
  ASSERT_EQ(expected_lines.size(), output_lines.size());
  for (std::size_t i = 0; i < expected_lines.size(); i++) {
    EXPECT_EQ(expected_lines[i], output_lines[i]);
  }
}

TEST(Format, JsonLoad) {
  Entity value = datapack::read_json<Entity>(entity_json);
  auto expected = Entity::example();
  ASSERT_EQ(value, expected);
}
