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
DATAPACK_TEMPLATED_INLINE(Point<T>, (typename T), x, y)
DATAPACK_TEMPLATED_INLINE((Pair<A, B>), (typename A, typename B), a, b)
DATAPACK_TEMPLATED_DECL((FooBar<Foo, Bar>), (typename Foo, typename Bar))
}
