#pragma once

#include <datapack/datapack.hpp>
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

namespace dpack {
DPACK_TEMPLATED_INLINE(Point<T>, (typename T), x, y)
DPACK_TEMPLATED_INLINE((Pair<A, B>), (typename A, typename B), a, b)
DPACK_TEMPLATED_DECL((FooBar<Foo, Bar>), (typename Foo, typename Bar))
} // namespace dpack

// Require custom type names for variants
DPACK_TYPE_NAMES(Point<double>, "Pointd", "pointd");
DPACK_TYPE_NAMES((Pair<int, double>), "NumberPair", "number_pair");
DPACK_TYPE_NAMES((FooBar<int, std::string>), "FooBar", "foo_bar");
