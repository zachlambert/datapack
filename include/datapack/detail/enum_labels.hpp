#pragma once

#include "datapack/detail/label.hpp"
#include "datapack/detail/nameof.hpp"
#include <array>
#include <optional>
#include <type_traits>
#include <utility>

/* Provides logic for automatically calculating the labels for an enum
 *
 * Inspired from: ttps://github.com/Neargye/magic_enum
 *
 */

namespace dpack {

template <typename E>
concept enum_c = std::is_enum_v<E>;

namespace detail {

// ==================================================================
// Query enum validity and size
// ie: Can integer x be converted to enum?
// How many enums exist over a given search range

// Default range to search, user may overload this to increase the range if required, but
// shouldn't be needed in 99.9% of use cases
template <enum_c E>
struct enum_search_range {
  static constexpr int start = 0;
  static constexpr int size = 256;
};

// Compile-time sequence of integers, which allows the search to occur at compile time
// https://en.cppreference.com/cpp/utility/integer_sequence
template <enum_c E>
using enum_search_sequence = std::make_integer_sequence<int, enum_search_range<E>::size>;

// For value name for invalid enums will be "(T)Value", eg: "(MyEnum)3"
// Can simply check if "(" or ")" is present in the unprocessed value name
template <enum_c E, int Value>
constexpr bool enum_value_valid() {
  return !enum_value_name<static_cast<E>(Value)>.contains('(');
}

// Iterate over the integer sequence and count the number of valid values using a fold operation
template <enum_c E, int... Is>
constexpr size_t get_enum_size(std::integer_sequence<int, Is...>) {
  return ((enum_value_valid<E, enum_search_range<E>::start + Is>() ? 1u : 0u) + ...);
}

} // namespace detail

template <enum_c E>
inline constexpr size_t enum_size = get_enum_size<E>(enum_search_sequence<E>());

// ==================================================================
// Build the enum label and value lists

namespace detail {

template <enum_c E, int Value>
constexpr void append_value(std::array<E, enum_size<E>>& result, size_t& count) {
  if constexpr (enum_value_valid<E, Value>()) {
    result[count] = static_cast<E>(Value);
    count++;
  }
}

template <enum_c E, int... Is>
constexpr std::array<E, enum_size<E>> get_enum_values(std::integer_sequence<int, Is...>) {
  std::array<E, enum_size<E>> values;
  size_t index = 0;
  (append_label<E, enum_search_range<E>::start + Is>(values, index), ...);
  return values;
}

template <enum_c E, int Value>
constexpr void append_label(std::array<std::string_view, enum_size<E>>& result, size_t& index) {
  if constexpr (enum_value_valid<E, Value>()) {
    result[index] = enum_label<static_cast<E>(Value)>();
    index++;
  }
}

template <enum_c E, int... Is>
constexpr std::array<std::string_view, enum_size<E>> get_enum_labels(
    std::integer_sequence<int, Is...>) {
  std::array<std::string_view, enum_size<E>> labels;
  size_t index = 0;
  (append_label<E, enum_search_range<E>::start + Is>(labels, index), ...);
  return labels;
}

} // namespace detail

// NOTE: Need to use "inline constexpr" if the address of enum_labels needs to taken (which it does)
// This forces the data to be stored in memory somewhere, which without inline would violate ODR
// since the symbol has external linkage and may be stored in multiple translation units
// Likewise for enum_values, etc

template <enum_c E>
inline constexpr std::array<std::string_view, enum_size<E>> enum_labels =
    detail::get_enum_labels<E>(detail::enum_search_sequence<E>());

template <enum_c E>
inline constexpr std::array<E, enum_size<E>> enum_values =
    detail::get_enum_values<E>(detail::enum_search_sequence<E>());

template <enum_c E>
constexpr std::optional<size_t> enum_index(const E& value) {
  for (size_t i = 0; i < enum_size<E>; i++) {
    if (enum_values<E>[i] == value) {
      return i;
    }
  }
  return std::nullopt;
}

template <enum_c E>
constexpr std::optional<E> enum_from_label(std::string_view label) {
  for (size_t i = 0; i < enum_size<E>; i++) {
    if (enum_labels<E>[i] == label) {
      return enum_values<E>[i];
    }
  }
  return std::nullopt;
}

template <enum_c E>
constexpr std::optional<std::string_view> enum_to_label(const E& value) {
  const auto index = enum_index(value);
  if (!index) {
    return std::nullopt;
  }
  return enum_labels<E>[*index];
}

} // namespace dpack
