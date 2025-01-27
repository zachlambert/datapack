#include <datapack/examples/entity.hpp>
#include <datapack/util/random.hpp>
#include <gtest/gtest.h>

TEST(Util, Random) {
  auto value = datapack::random<Entity>();
  // Simply check this runs without crashing
}
