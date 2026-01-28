#include <gtest/gtest.h>

#include <datapack/examples/entity.hpp>
#include <datapack/file.hpp>
#include <datapack/std/vector.hpp>
#include <filesystem>

TEST(File, WriteRead) {
  datapack::FileWriter writer("entity.dtp");
  writer.write("list", std::vector<int>{1, 2, 3});
  writer.write<std::string>("string", "hello");
  writer.write("entity", Entity::example());

  datapack::FileReader reader("entity.dtp");
  int i = 0;
  while (auto label = reader.next()) {
    if (label == "list") {
      EXPECT_EQ(i, 0);
      auto vector = reader.read<std::vector<int>>();
      ASSERT_EQ(vector.size(), 3);
      EXPECT_EQ(vector[0], 1);
      EXPECT_EQ(vector[1], 2);
      EXPECT_EQ(vector[2], 3);
    } else if (label == "string") {
      EXPECT_EQ(i, 1);
      auto string = reader.read<std::string>();
      EXPECT_EQ(string, "hello");
    } else if (label == "entity") {
      EXPECT_EQ(i, 2);
      auto entity = reader.read<Entity>();
      EXPECT_EQ(entity, Entity::example());
    } else {
      EXPECT_TRUE(false);
    }
    i++;
  }

  std::filesystem::remove("entity.dtp");
}
