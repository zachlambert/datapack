#include <datapack/examples/entity.hpp>
#include <datapack/random.hpp>
#include <gtest/gtest.h>

TEST(Random, RunsSuccessfully) {
  EXPECT_NO_THROW(datapack::random<Entity>());
  // Runs successfully
}
