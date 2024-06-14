#include <gtest/gtest.h>
#include <datapack/util/object_writer.hpp>
#include <datapack/examples/entity.hpp>

TEST(Object, Writer) {
    Entity entity = Entity::example();
    datapack::Object object = datapack::write_object(entity);

    // TODO
}
