#include "datapack/schema/schema.hpp"
#include "datapack/examples/entity.hpp"
#include "datapack/util/debug.hpp"
#include "datapack/util/random.hpp"
#include <gtest/gtest.h>

TEST(Schema, SchemaUsage) {
  datapack::Schema schema = datapack::create_schema<Entity>();

  std::stringstream ss;
  datapack::RandomReader reader;
  datapack::DebugWriter writer(ss);

  datapack::use_schema(schema, reader, writer);
  // No expect/assert, juts check it doesn't crash
}
