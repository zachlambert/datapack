#include <cmath>
#include <datapack/datapack.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/object.hpp>
#include <gtest/gtest.h>

TEST(Object, Edit) {
  using namespace datapack;

  Object object;
  object["a"] = 1.1;
  object["b"] = 2.2;

  auto c = object["c"];
  c.push_back("hello");
  c.push_back("world");

  object["d"] = true;

  auto e = object["e"];
  e["foo"] = "foo";
  e["bar"] = "bar";
  e["and"] = Object::null_t();

  EXPECT_EQ(object["a"].floating(), 1.1);
  EXPECT_EQ(object["b"].floating(), 2.2);
  EXPECT_EQ(object["c"][0].string(), "hello");
  EXPECT_EQ(object["c"][1].string(), "world");
  EXPECT_EQ(object["d"].boolean(), true);
  EXPECT_EQ(object["e"]["foo"].string(), "foo");
  EXPECT_EQ(object["e"]["bar"].string(), "bar");
  EXPECT_TRUE(object["e"]["and"].is_null());

  auto object_a = object["a"];
  EXPECT_EQ(object_a.floating(), 1.1);

  c[0] = "goodbye";
  EXPECT_EQ(object["c"][0].string(), "goodbye");

  auto e_clone = object["e"].clone();
  e_clone["and"] = "and";
  EXPECT_EQ(e_clone["and"].string(), "and");
  EXPECT_FALSE(object["e"]["and"].string_if());
  EXPECT_TRUE(object["e"]["and"].is_null());

  object["c"] = 3.3;
  object["e"] = false;
  EXPECT_EQ(object["c"].floating(), 3.3);
  EXPECT_EQ(object["e"].boolean(), false);
}

TEST(Object, Compare) {
  using namespace datapack;

  Object a;
  Object b;

  a.insert("x", 1.0);
  a.insert("y", 2.0);
  b.insert("y", 2.0);
  b.insert("x", 1.0);

  ASSERT_TRUE(a == b);
}

TEST(Object, Reader) {
  Entity in = Entity::example();

  datapack::Object object = datapack::write_object(in);
  Entity out = datapack::read_object<Entity>(object);

  EXPECT_EQ(in, out);
}

TEST(Object, Writer) {
  using namespace datapack;

  Entity entity = Entity::example();
  const Object object = write_object(entity);

  const Object expected = []() -> Object {
    Object expected;

    expected["index"] = 5;
    expected["name"] = "player";
    expected["enabled"] = true;

    auto pose = expected["pose"];
    pose["x"] = 1.0;
    pose["y"] = 1.0;
    pose["angle"] = M_PI / 2;

    expected["physics"] = "kinematic";

    auto hitbox = expected["hitbox"];
    hitbox["type"] = "circle";
    hitbox["value"]["radius"] = 1.0;

    auto sprite = expected["sprite"];
    sprite["width"] = 2;
    sprite["height"] = 2;
    auto sprite_data = sprite["data"];
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        auto pixel = *sprite_data.push_back(Object::map_t());
        pixel["r"] = (i + 0.5) / 2;
        pixel["g"] = (j + 0.5) / 2;
        pixel["b"] = 0.0;
      }
    }

    auto items = expected["items"];
    auto add_item = [&items](int count, const std::string& name) {
      auto item = *items.push_back(Object::map_t());
      item["count"] = count;
      item["name"] = name;
    };
    add_item(5, "hp_potion");
    add_item(1, "sword");
    add_item(1, "map");
    add_item(120, "gold");

    return expected;
  }();

  EXPECT_TRUE(object.at("index") == expected.at("index"));
  EXPECT_TRUE(object.at("name") == expected.at("name"));
  EXPECT_TRUE(object.at("enabled") == expected.at("enabled"));
  EXPECT_TRUE(object.at("pose") == expected.at("pose"));
  EXPECT_TRUE(object.at("physics") == expected.at("physics"));
  EXPECT_TRUE(object.at("hitbox") == expected.at("hitbox"));
  EXPECT_TRUE(object.at("sprite") == expected.at("sprite"));
  EXPECT_TRUE(object.at("items") == expected.at("items"));
  EXPECT_TRUE(object == expected);
}
