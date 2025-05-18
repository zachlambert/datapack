#include <datapack/debug.hpp>
#include <datapack/examples/entity.hpp>
#include <gtest/gtest.h>
#include <sstream>

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

TEST(Debug, ProducesCorrectOutput) {
  const std::string expected = R"((object) {
    index: 5,
    name: player,
    enabled: true,
    pose: (object) {
        x: 1,
        y: 2,
        angle: 3,
    },
    physics: (enum, 1 = kinematic),
    hitbox: (optional, has_value) {
        (variant, 0 = circle) {
            (object) {
                radius: 1,
            },
        },
    },
    sprite: (object) {
        width: 2,
        height: 2,
        data: (binary, length = 96),
    },
    items: (list) {
        (object) {
            count: 5,
            name: hp_potion,
        },
        (object) {
            count: 1,
            name: sword,
        },
        (object) {
            count: 1,
            name: map,
        },
        (object) {
            count: 120,
            name: gold,
        },
    },
    assigned_items: (tuple) {
        1,
        2,
        -1,
    },
},
)";

  std::stringstream ss;
  datapack::DebugWriter(ss).value(Entity::example());
  const std::string output = ss.str();

  auto expected_lines = get_lines(expected);
  auto output_lines = get_lines(output);
  ASSERT_EQ(expected_lines.size(), output_lines.size());
  for (std::size_t i = 0; i < expected_lines.size(); i++) {
    EXPECT_EQ(expected_lines[i], output_lines[i]);
  }
}
