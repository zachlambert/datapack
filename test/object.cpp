#include <cmath>
#include <datapack/datapack.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/object.hpp>
#include <gtest/gtest.h>

TEST(Object, ConstructFromPrimitive) {
  using namespace datapack;

  {
    Object object(1);
    ASSERT_TRUE(object.number_if());
    EXPECT_EQ(object.number(), 1.0);
  }

  {
    Object object(1.0);
    ASSERT_TRUE(object.number_if());
    EXPECT_EQ(object.number(), 1.0);
  }

  {
    Object object("hello");
    ASSERT_TRUE(object.string_if());
    EXPECT_EQ(object.string(), "hello");
  }

  {
    Object object(true);
    ASSERT_TRUE(object.boolean_if());
    EXPECT_EQ(object.boolean(), true);
  }

  {
    Object object(std::vector<std::uint8_t>{0xAA, 0xBB, 0xCC});
    ASSERT_TRUE(object.binary_if());
    EXPECT_EQ(object.binary(), std::vector<std::uint8_t>({0xAA, 0xBB, 0xCC}));
  }
}

TEST(Object, AssignToPrimitive) {
  using namespace datapack;

  {
    Object object;
    object = 1;
    ASSERT_TRUE(object.number_if());
    EXPECT_EQ(object.number(), 1.0);
  }

  {
    Object object;
    object = 1.0;
    ASSERT_TRUE(object.number_if());
    EXPECT_EQ(object.number(), 1.0);
  }

  {
    Object object;
    object = "hello";
    ASSERT_TRUE(object.string_if());
    EXPECT_EQ(object.string(), "hello");
  }

  {
    Object object;
    object = true;
    ASSERT_TRUE(object.boolean_if());
    EXPECT_EQ(object.boolean(), true);
  }

  {
    Object object;
    object = std::vector<std::uint8_t>{0xAA, 0xBB, 0xCC};
    ASSERT_TRUE(object.binary_if());
    EXPECT_EQ(object.binary(), std::vector<std::uint8_t>({0xAA, 0xBB, 0xCC}));
  }
}

TEST(Object, BuildMapWithOperatorSquareBrackets) {
  using namespace datapack;

  Object object;
  object["a"] = "a";
  object["b"] = "b";

  EXPECT_EQ(object["a"].string(), "a");
  EXPECT_EQ(object["b"].string(), "b");
}

TEST(Object, BuildMapWithInsert) {
  using namespace datapack;

  Object object;
  object.insert("a", "a");
  object.insert("b", "b");

  EXPECT_EQ(object["a"].string(), "a");
  EXPECT_EQ(object["b"].string(), "b");
  EXPECT_EQ(object.size(), 2);
}

TEST(Object, ConstructMapWithInitializerList) {
  using namespace datapack;

  ConstObject object = {{"a", "a"}, {"b", "b"}};

  ASSERT_TRUE(object.is_map());
  EXPECT_EQ(object["a"].string(), "a");
  EXPECT_EQ(object["b"].string(), "b");
  EXPECT_EQ(object.size(), 2);
}

TEST(Object, AssignToMapWithInitializerList) {
  using namespace datapack;

  Object object;
  object = {{"a", "a"}, {"b", "b"}};

  ASSERT_TRUE(object.is_map());
  EXPECT_EQ(object["a"].string(), "a");
  EXPECT_EQ(object["b"].string(), "b");
  EXPECT_EQ(object.size(), 2);
}

TEST(Object, BuildList) {
  using namespace datapack;

  Object object;
  object.push_back(10);
  object.push_back(20);
  object.push_back(30);

  ASSERT_TRUE(object.is_list());
  EXPECT_EQ(object[0].number(), 10);
  EXPECT_EQ(object[1].number(), 20);
  EXPECT_EQ(object[2].number(), 30);
  EXPECT_EQ(object.size(), 3);
}

TEST(Object, ConstructListFromInitializerList) {
  using namespace datapack;

  ConstObject object = {10, 20, 30};

  ASSERT_TRUE(object.is_list());
  EXPECT_EQ(object[0].number(), 10);
  EXPECT_EQ(object[1].number(), 20);
  EXPECT_EQ(object[2].number(), 30);
  EXPECT_EQ(object.size(), 3);
}

TEST(Object, AssignToListFromInitializerList) {
  using namespace datapack;

  Object object;
  object = {10, 20, 30};

  ASSERT_TRUE(object.is_list());
  EXPECT_EQ(object[0].number(), 10);
  EXPECT_EQ(object[1].number(), 20);
  EXPECT_EQ(object[2].number(), 30);
  EXPECT_EQ(object.size(), 3);
}

TEST(Object, ConstructMapOfObjectsWithInitializerList) {
  using namespace datapack;

  // NOTE: Need to wrap {true} to convert to an object
  ConstObject object = {
      {"foo", {{"a", "a"}, {"b", "b"}}},
      {"bar", Object(true)},
      {"baz", {1, 2, 3}}};

  std::cerr << object << std::endl;
  ASSERT_TRUE(object.is_map());
  ASSERT_TRUE(object.contains("foo"));
  ASSERT_TRUE(object.contains("bar"));
  ASSERT_TRUE(object.contains("baz"));

  ConstObject foo = object["foo"];
  ASSERT_TRUE(foo.is_map());
  ASSERT_TRUE(foo.contains("a") && foo["a"].string_if());
  ASSERT_TRUE(foo.contains("b") && foo["b"].string_if());
  EXPECT_EQ(foo["a"].string(), "a");
  EXPECT_EQ(foo["b"].string(), "b");

  ConstObject bar = object["bar"];
  ASSERT_TRUE(bar.boolean_if());
  EXPECT_EQ(bar.boolean(), true);

  ConstObject baz = object["baz"];
  ASSERT_TRUE(baz.is_list());
  ASSERT_EQ(baz.size(), 3);
  for (std::size_t i = 0; i < 3; i++) {
    ASSERT_TRUE(baz[i].number_if());
    ASSERT_EQ(baz[i].number(), i + 1);
  }
}

TEST(Object, ConstructListOfObjectsWithInitializerList) {
  using namespace datapack;

  ConstObject object = Object::make_list({Object("string"), {{"a", 10}, {"b", 20}}, {1, 2, 3}});

  ASSERT_TRUE(object.is_list());
  ASSERT_EQ(object.size(), 3);

  ASSERT_TRUE(object[0].string_if());
  ASSERT_EQ(object[0].string(), "string");

  ConstObject map = object[1];
  ASSERT_TRUE(map.is_map());
  ASSERT_TRUE(map.contains("a") && map["a"].number_if());
  ASSERT_EQ(map["a"].number(), 10);
  ASSERT_TRUE(map.contains("b") && map["b"].number_if());
  ASSERT_EQ(map["b"].number(), 20);

  ConstObject list = object[2];
  ASSERT_TRUE(list.is_list());
  ASSERT_EQ(list.size(), 3);
  for (std::size_t i = 0; i < 3; i++) {
    ASSERT_TRUE(list[i].number_if());
    ASSERT_EQ(list[i].number(), i + 1);
  }
}

#if 0
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

  EXPECT_EQ(object["a"].number(), 1.1);
  EXPECT_EQ(object["b"].number(), 2.2);
  EXPECT_EQ(object["c"][0].string(), "hello");
  EXPECT_EQ(object["c"][1].string(), "world");
  EXPECT_EQ(object["d"].boolean(), true);
  EXPECT_EQ(object["e"]["foo"].string(), "foo");
  EXPECT_EQ(object["e"]["bar"].string(), "bar");
  EXPECT_TRUE(object["e"]["and"].is_null());

  auto object_a = object["a"];
  EXPECT_EQ(object_a.number(), 1.1);

  c[0] = "goodbye";
  EXPECT_EQ(object["c"][0].string(), "goodbye");

  auto e_clone = object["e"].clone();
  e_clone["and"] = "and";
  EXPECT_EQ(e_clone["and"].string(), "and");
  EXPECT_FALSE(object["e"]["and"].string_if());
  EXPECT_TRUE(object["e"]["and"].is_null());

  object["c"] = 3.3;
  object["e"] = false;
  EXPECT_EQ(object["c"].number(), 3.3);
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
  entity.sprite.data.clear(); // Ignore data
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
    sprite["data"] = ""; // Removed data

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
#endif
