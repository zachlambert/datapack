#include <datapack/binary.hpp>
#include <datapack/examples/entity.hpp>
#include <gtest/gtest.h>

TEST(Binary, WriteRead) {
  Entity in = Entity::example();

  std::vector<std::uint8_t> data = datapack::to_binary(in);
  Entity out = datapack::from_binary<Entity>(data);

  ASSERT_EQ(in, out);
}
