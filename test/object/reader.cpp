#include <datapack/examples/entity.hpp>
#include <datapack/util/debug.hpp>
#include <datapack/util/object_reader.hpp>
#include <datapack/util/object_writer.hpp>
#include <gtest/gtest.h>

TEST(Object, Reader) {
  Entity in = Entity::example();

  datapack::Object object = datapack::write_object(in);
  Entity out = datapack::read_object<Entity>(object);

  EXPECT_EQ(in, out);
}
