#pragma once

#include "datapack/detail/name_utils.hpp"
#include <string_view>

namespace dpack {

template <typename T>
struct TypeNames {
  static constexpr std::string_view name =
      name_utils::remove_scope(name_utils::type_name_full<T>());
  static_assert(name_utils::is_simple_name(name));

  static constexpr auto label_value = name_utils::name_to_label<name_utils::label_size(name)>(name);
  static constexpr std::string_view label = {label_value.data(), label_value.size()};
};

#define DPACK_TYPE_NAMES(T, name_str, label_str)                                                   \
  template <>                                                                                      \
  struct TypeNames<T> {                                                                            \
    static constexpr std::string_view name = name_str;                                             \
    static constexpr std::string_view label = label_str;                                           \
  }

template <typename T>
constexpr std::string_view type_name() {
  return TypeNames<T>::name;
}

template <typename T>
constexpr std::string_view type_label() {
  return TypeNames<T>::label;
}

} // namespace dpack
