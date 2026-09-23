#pragma once

#include "datapack/names/name_utils.hpp"
#include <string_view>

namespace dpack {

namespace detail {

template <typename T>
struct TypeNames {
  static constexpr std::string_view name = remove_scope(type_name_full<T>());
  static_assert(is_simple_name(name));

  static constexpr auto label_value = name_to_label<label_size(name)>(name);
  static constexpr std::string_view label = {label_value.data(), label_value.size()};
};

} // namespace detail

#define DPACK_TYPE_NAMES(T, name_str, label_str)                                                   \
  template <>                                                                                      \
  struct dpack::detail::TypeNames<T> {                                                             \
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
