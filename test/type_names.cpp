#include <datapack/detail/type_names.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace dpack;

namespace ns {

struct Foo {};

template <typename T>
struct Bar {
  T x;
};

} // namespace ns

namespace dpack {
DPACK_TYPE_NAMES(ns::Bar<int>, "Bari", "bari");
DPACK_TYPE_NAMES(ns::Bar<double>, "Bard", "bard");
} // namespace dpack

TEST(NameOf, TypeNames) {
  EXPECT_EQ(type_name<int>(), "int");
  EXPECT_EQ(type_label<int>(), "int");
  EXPECT_EQ(type_name<ns::Foo>(), "Foo");
  EXPECT_EQ(type_label<ns::Foo>(), "foo");
  EXPECT_EQ(type_name<ns::Bar<int>>(), "Bari");
  EXPECT_EQ(type_label<ns::Bar<int>>(), "bari");
  EXPECT_EQ(type_name<ns::Bar<double>>(), "Bard");
  EXPECT_EQ(type_label<ns::Bar<double>>(), "bard");
}
