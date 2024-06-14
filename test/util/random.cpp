#include <gtest/gtest.h>
#include <datapack/util/random.hpp>
#include <datapack/examples/entity.hpp>

TEST(Util, Random) {
    auto value = datapack::random<Entity>();
    // Simply check this runs without crashing
}
