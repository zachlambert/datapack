#include <datapack/detail/nameof.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace dpack;

namespace ns {

struct Foo {};

enum class Enum { First, Second };

} // namespace ns

TEST(NameOf, TypeNames) {
  EXPECT_EQ(detail::type_name<int>(), "int");
  EXPECT_EQ(default_type_name<int>(), "int");
  EXPECT_EQ(default_type_label<int>(), "int");

  EXPECT_EQ(detail::type_name<ns::Foo>(), "ns::Foo");
  EXPECT_EQ(default_type_name<ns::Foo>(), "Foo");
  EXPECT_EQ(default_type_label<ns::Foo>(), "foo");
}

TEST(NameOf, EnumValueNames) {
  EXPECT_EQ(detail::enum_value_name<ns::Enum::First>(), "ns::Enum::First");
  EXPECT_EQ(default_enum_value_name<ns::Enum::First>(), "First");
  EXPECT_EQ(default_enum_value_label<ns::Enum::First>(), "first");

  EXPECT_EQ(detail::enum_value_name<ns::Enum::Second>(), "ns::Enum::Second");
  EXPECT_EQ(default_enum_value_name<ns::Enum::Second>(), "Second");
  EXPECT_EQ(default_enum_value_label<ns::Enum::Second>(), "second");
}
