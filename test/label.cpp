#include "datapack/label.hpp"

#include <gtest/gtest.h>
#include <string>

using namespace dpack;

namespace ns {

struct Foo {};
struct Bar {};

enum class Enum { First, Second };

template <typename A, typename B>
struct Baz {
  A a;
  B b;
};

template <typename T>
struct Point {
  T x;
  T y;
};

} // namespace ns

namespace dpack {

DPACK_TYPE_LABEL(ns::Bar, "barrr");
DPACK_ENUM_VALUE_LABEL(ns::Enum::Second, "sec");
DPACK_TYPE_LABEL(ns::Point<float>, "pointf");
DPACK_TYPE_LABEL(ns::Point<double>, "pointd");

} // namespace dpack

TEST(Label, Label) {
  EXPECT_EQ(type_label<int>(), "int");
  EXPECT_EQ(type_label<ns::Foo>(), "foo");
  EXPECT_EQ(type_label<ns::Bar>(), "barrr");

  EXPECT_EQ(enum_value_label<ns::Enum::First>(), "first");
  EXPECT_EQ(enum_value_label<ns::Enum::Second>(), "sec");

  // By default, excludes the template arguments from the label
  EXPECT_EQ((type_label<ns::Baz<int, double>>()), "baz");

  // If the template argument/s matter, have to define custom labels
  EXPECT_EQ(type_label<ns::Point<float>>(), "pointf");
  EXPECT_EQ(type_label<ns::Point<double>>(), "pointd");
}
