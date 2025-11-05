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

  ConstObject object = {
      {"foo", {{"a", "a"}, {"b", "b"}}},
      {"bar", Object(true)},
      {"baz", {1, 2, 3}}};

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

TEST(Object, OverwriteToPrimitive) {
  using namespace datapack;

  Object object;
  object = 12;
  EXPECT_EQ(object.number(), 12);
  object = "Hello";
  ASSERT_FALSE(object.number_if());
  EXPECT_EQ(object.string(), "Hello");

  EXPECT_THROW(object["a"] = 1, TypeError);
  object.to_map();
  object["a"] = 1;
  ASSERT_TRUE(object.is_map());

  EXPECT_THROW(object.push_back(1), TypeError);
  object.to_list();
  object.push_back(1);
  ASSERT_TRUE(object.is_list());

  ASSERT_TRUE(object[0].number_if());
  object[0].to_map();
  ASSERT_TRUE(object[0].is_map());
}

TEST(Object, CloneWillNotModifyOriginal) {
  using namespace datapack;

  Object object = {{"a", 12}, {"b", 24}};
  Object copy = object.clone();
  copy["b"] = 36;
  EXPECT_EQ(object["b"].number(), 24);
  EXPECT_EQ(copy["b"].number(), 36);
}

TEST(Object, CopyWillModifyOriginal) {
  using namespace datapack;

  Object object = {{"a", 12}, {"b", 24}};
  Object copy = object;
  copy["b"] = 36;
  EXPECT_EQ(object["b"].number(), 36);
  EXPECT_EQ(copy["b"].number(), 36);
}

TEST(Object, AccessViaAtWillNotModify) {
  using namespace datapack;

  Object object;
  EXPECT_THROW(object.at("a"), TypeError);
  object.to_map();
  EXPECT_THROW(object.at("a"), KeyError);
}

TEST(Object, AccessViaOperatorSquareBracketWillThrowForConstObject) {
  using namespace datapack;

  Object object;
  object["a"] = 12;

  ConstObject const_object = object;
  EXPECT_NO_THROW(const_object["a"]);
  EXPECT_THROW(const_object["b"], KeyError);

  EXPECT_NO_THROW(object["a"]);
  EXPECT_NO_THROW(object["b"]);
}

TEST(Object, DuplicateInsertionWillThrow) {
  using namespace datapack;

  Object object;
  object.insert("a", "Hello");
  EXPECT_THROW(object.insert("a", "Again"), KeyError);
}

TEST(Object, ObjectContains) {
  using namespace datapack;

  ConstObject map = {{"a", 1}, {"b", 2}};
  EXPECT_TRUE(map.contains("a"));
  EXPECT_TRUE(map.contains("b"));
  EXPECT_FALSE(map.contains("c"));

  ConstObject list = {1, 2, 3};
  EXPECT_THROW(list.contains("a"), TypeError);
}

TEST(Object, ObjectFindReturnsHandle) {
  using namespace datapack;

  ConstObject map = {{"a", 1}, {"b", 2}};
  EXPECT_TRUE(map.find("a") && map.find("a")->number() == 1);
  EXPECT_TRUE(map.find("b") && map.find("b")->number() == 2);
  EXPECT_FALSE(map.find("c"));

  ConstObject list = {1, 2, 3};
  EXPECT_THROW(list.find("a"), TypeError);
}

TEST(Object, ObjectEraseRemovesAndDecreasesSize) {
  using namespace datapack;

  Object object = {{"a", 1}, {"b", 2}, {"c", 2}};
  ASSERT_TRUE(object.contains("b") && object.at("b") == 2);
  EXPECT_EQ(object.size(), 3);

  object["b"].erase();
  EXPECT_FALSE(object.contains("b"));
  EXPECT_EQ(object.size(), 2);
}

TEST(Object, ObjectCanIterateOverListValues) {
  using namespace datapack;

  Object object = {1, 2, 3};
  {
    int i = 1;
    for (auto value : object.values()) {
      EXPECT_EQ(value, i);
      i++;
      value.number()++;
    }
  }
  {
    int i = 2;
    for (auto value : object.values()) {
      EXPECT_EQ(value, i);
      i++;
    }
  }

  ConstObject const_object = {1, 2, 3};
  {
    int i = 1;
    for (auto value : const_object.values()) {
      EXPECT_EQ(value, i);
      i++;
    }
  }

  EXPECT_THROW(object.items().begin(), TypeError);
  EXPECT_THROW(object.items().end(), TypeError);
}

TEST(Object, ObjectCanIterateOverMapValuesAndItems) {
  using namespace datapack;

  Object object = {{"a", 1}, {"b", 2}, {"c", 3}};
  {
    std::string letter = "a";
    int i = 1;
    for (auto [key, value] : object.items()) {
      EXPECT_EQ(key, letter);
      key[0] = std::toupper(key[0]);
      value.number()++;
      i++;
      letter[0]++;
    }
  }
  {
    std::string letter = "A";
    int i = 2;
    for (auto [key, value] : object.items()) {
      EXPECT_EQ(key, letter);
      EXPECT_EQ(value, i);
      i++;
      letter[0]++;
    }
  }
  {
    int i = 2;
    for (auto value : object.values()) {
      EXPECT_EQ(value, i);
      i++;
    }
  }

  ConstObject const_object = {{"a", 1}, {"b", 2}, {"c", 3}};
  {
    std::string letter = "a";
    int i = 1;
    for (auto [key, value] : const_object.items()) {
      EXPECT_EQ(key, letter);
      EXPECT_EQ(value, i);
      i++;
      letter[0]++;
    }
  }
  {
    int i = 1;
    for (auto value : const_object.values()) {
      EXPECT_EQ(value, i);
      i++;
    }
  }
}

TEST(Object, CanCompareObjectAndPrimitive) {
  using namespace datapack;

  ConstObject a(1);

  EXPECT_TRUE(a == 1);
  EXPECT_TRUE(1 == a);
  EXPECT_FALSE(a != 1);
  EXPECT_FALSE(1 != a);

  EXPECT_FALSE(a == "hello");
  EXPECT_FALSE("hello" == a);
  EXPECT_TRUE("hello" != a);
  EXPECT_TRUE(a != "hello");
}

TEST(Object, CanCompareObjects) {
  using namespace datapack;

  ConstObject one = {
      {"a", {1, 2, 3}},
      {"b", Object(false)},
      {"c",
       {
           {"foo", "FOO"},
           {"bar", "BAR"},
       }},
      {"d", Object(std::vector<std::uint8_t>{0x01, 0x23, 0x45})}};

  Object two;
  two["a"] = {1, 2}, two.at("a").push_back(3);
  two.insert("b", false);
  two["c"].to_map();
  two.at("c")["foo"] = "FOO";
  two.at("c")["bar"] = "BAR";
  two["d"] = std::vector<std::uint8_t>{0x01, 0x23, 0x45};
  EXPECT_TRUE(one == two);
  EXPECT_FALSE(one != two);

  ConstObject three = {
      {"a", {1, 2, 3}},
      {"b", Object(false)},
      {"c",
       {
           {"fo", "FOO"},
           {"bar", "BAR"},
       }},
      {"d", Object(std::vector<std::uint8_t>{0x01, 0x23, 0x45})}};
  EXPECT_FALSE(one == three);
  EXPECT_TRUE(one != three);

  ConstObject four = {
      {"a", {1, 2, 3, 4}},
      {"b", Object(false)},
      {"c",
       {
           {"foo", "FOO"},
           {"bar", "BAR"},
       }},
      {"d", Object(std::vector<std::uint8_t>{0x01, 0x23, 0x45})}};
  EXPECT_FALSE(one == four);
  EXPECT_TRUE(one != four);

  ConstObject five = {
      {"a", {1, 2, 3}},
      {"b", Object(0.0)},
      {"c",
       {
           {"foo", "FOO"},
           {"bar", "BAR"},
       }},
      {"d", Object(std::vector<std::uint8_t>{0x01, 0x23, 0x45})}};
  EXPECT_FALSE(one == five);
  EXPECT_TRUE(one != five);

  ConstObject six = {
      {"a", {1, 2, 3}},
      {"b", Object(0.0)},
      {"c",
       {
           {"foo", "FOO"},
           {"bar", "BAR"},
       }},
      {"d", Object(std::vector<std::uint8_t>{0x01, 0x23, 0x45})},
      {"e", Object("another")}};
  EXPECT_FALSE(one == six);
  EXPECT_TRUE(one != six);
}

TEST(Object, CanTraverseWithHandle) {
  using namespace datapack;

  ConstObject object = {
      {"a", {1, 2, 3}},
      {"b", Object(false)},
      {"c",
       {
           {"foo", "FOO"},
           {"bar", "BAR"},
       }},
      {"d", Object(std::vector<std::uint8_t>{0x01, 0x23, 0x45})}};

  ConstNodeHandle iter = object.handle();
  ASSERT_TRUE(iter);
  ASSERT_TRUE(iter->is_map());

  ConstNodeHandle a = iter.child();
  ASSERT_TRUE(a);
  EXPECT_EQ(a.key(), "a");

  ConstNodeHandle b = a.next();
  ASSERT_TRUE(b);
  EXPECT_EQ(b.key(), "b");

  ConstNodeHandle c = b.next();
  ASSERT_TRUE(c);
  EXPECT_EQ(c.key(), "c");

  ConstNodeHandle d = c.next();
  ASSERT_TRUE(d);
  EXPECT_EQ(d.key(), "d");

  EXPECT_FALSE(d.next());

  {
    ASSERT_TRUE(a->is_list());

    auto iter = a.child();
    ASSERT_TRUE(iter);
    EXPECT_EQ(*iter, 1);
    EXPECT_EQ(iter.key(), "");
    EXPECT_FALSE(iter.child());

    iter = iter.next();
    ASSERT_TRUE(iter);
    EXPECT_EQ(*iter, 2);
    EXPECT_EQ(iter.key(), "");
    EXPECT_FALSE(iter.child());

    iter = iter.next();
    ASSERT_TRUE(iter);
    EXPECT_EQ(*iter, 3);
    EXPECT_EQ(iter.key(), "");
    EXPECT_FALSE(iter.child());

    EXPECT_FALSE(iter.next());
  }

  {
    ASSERT_TRUE(b->boolean_if());
    EXPECT_EQ(*b, false);
    EXPECT_FALSE(b.child());
  }

  {
    ASSERT_TRUE(c->is_map());

    auto iter = c.child();
    ASSERT_TRUE(iter);
    EXPECT_EQ(*iter, "FOO");
    EXPECT_EQ(iter.key(), "foo");
    EXPECT_FALSE(iter.child());

    iter = iter.next();
    ASSERT_TRUE(iter);
    EXPECT_EQ(*iter, "BAR");
    EXPECT_EQ(iter.key(), "bar");
    EXPECT_FALSE(iter.child());

    EXPECT_FALSE(iter.next());
  }

  {
    ASSERT_TRUE(d->binary_if());
    EXPECT_FALSE(d.child());
    EXPECT_FALSE(d.next());
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
