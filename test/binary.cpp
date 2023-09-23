#include <datapack/binary.hpp>
#include <datapack/examples/entity.hpp>
#include <gtest/gtest.h>

TEST(Format, Binary) {
  Entity in = Entity::example();

  std::vector<std::uint8_t> data = datapack::write_binary(in);
  Entity out = datapack::read_binary<Entity>(data);

  ASSERT_EQ(in, out);
}
