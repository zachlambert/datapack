#include <gtest/gtest.h>
#include <datapack/util/object_writer.hpp>
#include <datapack/util/object_reader.hpp>
#include <datapack/examples/entity.hpp>

TEST(Object, Reader) {
    Entity in = Entity::example();

    datapack::Object object = datapack::write_object(in);
    Entity out = datapack::read_object<Entity>(object);

    EXPECT_TRUE(compare(in, out));
}
