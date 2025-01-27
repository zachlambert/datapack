#include <datapack/examples/entity.hpp>
#include <datapack/format/binary_reader.hpp>
#include <datapack/format/binary_writer.hpp>
#include <datapack/schema/binary.hpp>
#include <datapack/util/object_reader.hpp>
#include <datapack/util/object_writer.hpp>
#include <datapack/util/random.hpp>
#include <gtest/gtest.h>

TEST(Schema, ObjectToBinary) {
  auto schema = datapack::create_schema<Entity>();

  Entity value = datapack::random<Entity>();
  datapack::Object object = datapack::write_object(value);

  std::vector<std::uint8_t> out_direct = datapack::write_binary(value);
  std::vector<std::uint8_t> out_schema = datapack::object_to_binary(schema, object);

  ASSERT_EQ(out_direct.size(), out_schema.size());
  for (std::size_t i = 0; i < out_direct.size(); i++) {
    EXPECT_EQ(out_direct[i], out_schema[i]);
  }
}

TEST(Schema, BinaryToObject) {
  auto schema = datapack::create_schema<Entity>();

  Entity value = datapack::random<Entity>();
  std::vector<std::uint8_t> bytes = datapack::write_binary(value);

  datapack::Object out_direct = datapack::write_object(value);
  datapack::Object out_schema = datapack::binary_to_object(schema, bytes);

  EXPECT_EQ(out_direct, out_schema);
}
