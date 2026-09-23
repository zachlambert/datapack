#include "datapack/names/enum_names.hpp"
#include "datapack/names/type_names.hpp"
#include "datapack/names/variant_names.hpp"
#include "datapack/std/variant.hpp"
#include <gtest/gtest.h>

using namespace dpack;

namespace ns {

struct Foo {};

template <typename T>
struct Bar {
  T x;
};

enum class Enum { First, Second };
enum class Enum2 { Invalid_Name, Another };

using Variant = std::variant<Foo, Bar<int>, Bar<double>>;

} // namespace ns

DPACK_TYPE_NAMES(ns::Variant, "Variant", "variant");
DPACK_TYPE_NAMES(ns::Bar<int>, "Bari", "bari");
DPACK_TYPE_NAMES(ns::Bar<double>, "Bard", "bard");
DPACK_ENUM_VALUE_NAMES(ns::Enum2, Invalid_Name, "InvalidName", "invalid_name");

TEST(Names, TypeNames) {
  EXPECT_EQ(type_name<int>(), "int");
  EXPECT_EQ(type_label<int>(), "int");
  EXPECT_EQ(type_name<ns::Foo>(), "Foo");
  EXPECT_EQ(type_label<ns::Foo>(), "foo");
  EXPECT_EQ(type_name<ns::Bar<int>>(), "Bari");
  EXPECT_EQ(type_label<ns::Bar<int>>(), "bari");
  EXPECT_EQ(type_name<ns::Bar<double>>(), "Bard");
  EXPECT_EQ(type_label<ns::Bar<double>>(), "bard");
}

TEST(Names, EnumValueNames) {
  EXPECT_EQ(type_name<ns::Enum>(), "Enum");
  EXPECT_EQ(type_label<ns::Enum>(), "enum");
  EXPECT_EQ(enum_value_name<ns::Enum::First>(), "First");
  EXPECT_EQ(enum_value_label<ns::Enum::First>(), "first");
  EXPECT_EQ(enum_value_name<ns::Enum::Second>(), "Second");
  EXPECT_EQ(enum_value_label<ns::Enum::Second>(), "second");

  EXPECT_EQ(type_name<ns::Enum2>(), "Enum2");
  EXPECT_EQ(type_label<ns::Enum2>(), "enum2");
  EXPECT_EQ(enum_value_name<ns::Enum2::Invalid_Name>(), "InvalidName");
  EXPECT_EQ(enum_value_label<ns::Enum2::Invalid_Name>(), "invalid_name");
  EXPECT_EQ(enum_value_name<ns::Enum2::Another>(), "Another");
  EXPECT_EQ(enum_value_label<ns::Enum2::Another>(), "another");
}

TEST(Names, EnumQueries) {
  EXPECT_EQ(enum_size<ns::Enum>, 2);
  EXPECT_EQ(enum_size<ns::Enum2>, 2);

  {
    auto value = enum_from_label<ns::Enum>("first");
    ASSERT_TRUE(value);
    EXPECT_EQ(*value, ns::Enum::First);
  }
  {
    auto value = enum_from_label<ns::Enum>("second");
    ASSERT_TRUE(value);
    EXPECT_EQ(*value, ns::Enum::Second);
  }
  {
    auto value = enum_from_label<ns::Enum>("Second");
    EXPECT_FALSE(value);
  }
  {
    auto value = enum_from_label<ns::Enum>("third");
    EXPECT_FALSE(value);
  }

  {
    auto labels = enum_labels<ns::Enum>;
    ASSERT_EQ(labels.size(), 2);
    EXPECT_EQ(labels[0], "first");
    EXPECT_EQ(labels[1], "second");
  }

  EXPECT_EQ(enum_index(ns::Enum::First), 0);
  EXPECT_EQ(enum_index(ns::Enum::Second), 1);
  EXPECT_EQ(enum_to_label(ns::Enum::First), "first");
  EXPECT_EQ(enum_to_label(ns::Enum::Second), "second");

  {
    auto value = enum_from_label<ns::Enum2>("invalid_name");
    ASSERT_TRUE(value);
    EXPECT_EQ(*value, ns::Enum2::Invalid_Name);
  }
  {
    auto value = enum_from_label<ns::Enum2>("another");
    ASSERT_TRUE(value);
    EXPECT_EQ(*value, ns::Enum2::Another);
  }
  {
    auto value = enum_from_label<ns::Enum2>("Second");
    EXPECT_FALSE(value);
  }
  {
    auto value = enum_from_label<ns::Enum2>("third");
    EXPECT_FALSE(value);
  }

  {
    auto labels = enum_labels<ns::Enum2>;
    ASSERT_EQ(labels.size(), 2);
    EXPECT_EQ(labels[0], "invalid_name");
    EXPECT_EQ(labels[1], "another");
  }

  EXPECT_EQ(enum_index(ns::Enum2::Invalid_Name), 0);
  EXPECT_EQ(enum_index(ns::Enum2::Another), 1);
  EXPECT_EQ(enum_to_label(ns::Enum2::Invalid_Name), "invalid_name");
  EXPECT_EQ(enum_to_label(ns::Enum2::Another), "another");
}

TEST(Names, VariantValueNames) {
  EXPECT_EQ(type_name<ns::Variant>(), "Variant");
  EXPECT_EQ(type_label<ns::Variant>(), "variant");
  EXPECT_EQ(enum_value_name<ns::Enum::First>(), "First");
  EXPECT_EQ(enum_value_label<ns::Enum::First>(), "first");
  EXPECT_EQ(enum_value_name<ns::Enum::Second>(), "Second");
  EXPECT_EQ(enum_value_label<ns::Enum::Second>(), "second");

  EXPECT_EQ(type_name<ns::Enum2>(), "Enum2");
  EXPECT_EQ(type_label<ns::Enum2>(), "enum2");
  EXPECT_EQ(enum_value_name<ns::Enum2::Invalid_Name>(), "InvalidName");
  EXPECT_EQ(enum_value_label<ns::Enum2::Invalid_Name>(), "invalid_name");
  EXPECT_EQ(enum_value_name<ns::Enum2::Another>(), "Another");
  EXPECT_EQ(enum_value_label<ns::Enum2::Another>(), "another");
}

TEST(Names, VariantQueries) {
  using ns::Variant;

  EXPECT_EQ(variant_size<Variant>, 3);

  {
    auto labels = variant_labels<Variant>;
    ASSERT_EQ(labels.size(), 3);
    EXPECT_EQ(labels[0], "foo");
    EXPECT_EQ(labels[1], "bari");
    EXPECT_EQ(labels[2], "bard");
  }

  EXPECT_EQ(variant_index(Variant(ns::Foo())), 0);
  EXPECT_EQ(variant_index(Variant(ns::Bar<int>())), 1);
  EXPECT_EQ(variant_index(Variant(ns::Bar<double>())), 2);

  EXPECT_EQ(variant_to_label(Variant(ns::Foo())), "foo");
  EXPECT_EQ(variant_to_label(Variant(ns::Bar<int>())), "bari");
  EXPECT_EQ(variant_to_label(Variant(ns::Bar<double>())), "bard");

  {
    auto value = variant_from_label<Variant>("foo");
    ASSERT_TRUE(value);
    EXPECT_EQ(value->index(), 0);
  }
  {
    auto value = variant_from_label<Variant>("bard");
    ASSERT_TRUE(value);
    EXPECT_EQ(value->index(), 2);
  }
  {
    auto value = variant_from_label<Variant>("Foo");
    EXPECT_FALSE(value);
  }
  {
    auto value = variant_from_label<Variant>("baz");
    EXPECT_FALSE(value);
  }
}
