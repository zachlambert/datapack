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
