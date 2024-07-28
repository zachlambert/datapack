#include <gtest/gtest.h>
#include <datapack/object.hpp>

TEST(Object, Edit) {
    using namespace datapack;

    Object object;
    object["a"] = "hello";

#if 0
    Object object = Object(Object::map_t());

    object.insert("a", 1.1);
    object.insert("b", 2.2);

    auto c = object.insert("c", Object::list_t());
    c.append("hello");
    c.append("world");

    object.insert("d", true);
    auto e = object.insert("e", Object::map_t());
    e.insert("foo", "foo");
    e.insert("bar", "bar");
    e.insert("and", std::nullopt);

    EXPECT_EQ(object["a"].get<double>(), 1.1);
    EXPECT_EQ(object["b"].get<double>(), 2.2);
    EXPECT_EQ(object["c"][0].get<std::string>(), "hello");
    EXPECT_EQ(object["c"][1].get<std::string>(), "world");
    EXPECT_EQ(object["d"].get<bool>(), true);
    EXPECT_EQ(object["e"]["foo"].get<std::string>(), "foo");
    EXPECT_EQ(object["e"]["bar"].get<std::string>(), "bar");
    EXPECT_TRUE(object["e"]["and"].get_if<std::nullopt_t>());

    auto object_a = object["a"];
    EXPECT_EQ(object_a.get<double>(), 1.1);

    c[0].set("goodbye");
    EXPECT_EQ(object["c"][0].get<std::string>(), "goodbye");

    auto e_clone = object["e"].clone();
    e_clone["and"].set("and");
    EXPECT_EQ(e_clone["and"].get<std::string>(), "and");
    EXPECT_FALSE(object["e"]["and"].get_if<std::string>());
    EXPECT_FALSE(object["e"]["and"].get_if<std::string>());
    EXPECT_TRUE(object["e"]["and"].get_if<std::nullopt_t>());

    object["c"].set(3.3);
    object["e"].set(false);
    EXPECT_EQ(object["c"].get<double>(), 3.3);
    EXPECT_EQ(object["e"].get<bool>(), false);
#endif
}

TEST(Object, Compare) {
    using namespace datapack;

#if 0
    Object a = Object(Object::map_t());
    Object b = Object(Object::map_t());

    a.insert("x", 1.0);
    a.insert("y", 2.0);
    b.insert("y", 2.0);
    b.insert("x", 1.0);

    ASSERT_TRUE(compare(a, b));
#endif
}
