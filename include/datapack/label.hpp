#pragma once

#include "datapack/detail/nameof.hpp"
#include <string_view>

namespace dpack {

namespace detail {

template <typename T>
struct TypeLabel {
  static constexpr std::string_view value = default_type_label<T>();
};

template <auto V>
struct EnumValueLabel {
  static constexpr std::string_view value = default_enum_value_label<V>();
};

} // namespace detail

#define DPACK_TYPE_LABEL(T, label)                                                                 \
  template <>                                                                                      \
  struct detail::TypeLabel<T> {                                                                    \
    static constexpr std::string_view value = label;                                               \
  }

#define DPACK_ENUM_VALUE_LABEL(V, label)                                                           \
  template <>                                                                                      \
  struct detail::EnumValueLabel<V> {                                                               \
    static constexpr std::string_view value = label;                                               \
  }

template <typename T>
constexpr std::string_view type_label() {
  return detail::TypeLabel<T>::value;
}

template <auto V>
requires std::is_enum_v<decltype(V)>
constexpr std::string_view enum_value_label() {
  return detail::EnumValueLabel<V>::value;
}

} // namespace dpack
