#pragma once

#include "datapack/names/name_utils.hpp"
#include <string_view>

// Moved from datapack.hpp since _DPACK_DPAREN is required for DPACK_TYPE_NAMES
// https://stackoverflow.com/a/62984543
// Needs to support both:
//   _DPACK_DEPAREN((x, y)) -> x, y   Does have parentheses
//   _DPACK_DEPAREN(x)      -> x      Doesn't have parentheses
// Required to pass a type with more than one template argument to a macro, since the comma in
// eg: "Pair<int, double>" would otherwise be read as an argument separator
#define _DPACK_DEPAREN(X) _DPACK_REMOVE_DP_PAREN(_DP_PAREN X)
#define _DP_PAREN(...) _DP_PAREN __VA_ARGS__
#define _DPACK_REMOVE_DP_PAREN(...) _DPACK_REMOVE_DP_PAREN2(__VA_ARGS__)
#define _DPACK_REMOVE_DP_PAREN2(...) _IGNORE##__VA_ARGS__
#define _IGNORE_DP_PAREN

namespace dpack {

namespace detail {

template <typename T>
struct TypeNames {
  static constexpr std::string_view name = remove_scope(type_name_full<T>());
  static_assert(
      is_simple_name(name),
      "Automatic type names are only defined for 'simple' type names, must use DPACK_TYPE_NAMES to "
      "define a custom type name and label");

  static constexpr auto label_value = name_to_label<label_size(name)>(name);
  static constexpr std::string_view label = {label_value.data(), label_value.size()};
};

} // namespace detail

// A templated type must be parenthesised if it has more than one template argument,
// eg: DPACK_TYPE_NAMES((Pair<int, double>), "Pairid", "pairid")
#define DPACK_TYPE_NAMES(T, name_str, label_str)                                                   \
  template <>                                                                                      \
  struct dpack::detail::TypeNames<_DPACK_DEPAREN(T)> {                                             \
    static constexpr std::string_view name = name_str;                                             \
    static constexpr std::string_view label = label_str;                                           \
  }

template <typename T>
constexpr std::string_view type_name() {
  return ::dpack::detail::TypeNames<T>::name;
}

template <typename T>
constexpr std::string_view type_label() {
  return ::dpack::detail::TypeNames<T>::label;
}

} // namespace dpack
