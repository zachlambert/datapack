#pragma once

#include <datapack/datapack.hpp>
#include <datapack/labelled_enum.hpp>
#include <datapack/labelled_variant.hpp>
#include <datapack/std/string.hpp>

template <typename T>
struct Point {
  T x;
  T y;
};

template <typename A, typename B>
struct Pair {
  A a;
  B b;
};

template <typename Foo, typename Bar>
struct FooBar {
  Foo foo;
  Bar bar;
};

namespace datapack {
DATAPACK_TEMPLATED_INLINE((typename T), (Point<T>), x, y)
DATAPACK_TEMPLATED_INLINE((typename A, typename B), (Pair<A, B>), a, b)
DATAPACK_TEMPLATED_DECL((typename Foo, typename Bar), (FooBar<Foo, Bar>))
}
