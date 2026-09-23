#pragma once

#include "datapack/names/type_names.hpp"
#include <array>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

/* Provides logic for automatically calculating the labels for a variant
 *
 * Unlike an enum, there is nothing to search for: every alternative of a variant is a type, so the
 * labels are simply the type labels of the alternatives, looked up with std::variant_alternative_t.
 *
 * This also means there is no variant equivalent of DPACK_ENUM_VALUE_NAMES. A label is customised
 * by using DPACK_TYPE_NAMES on the alternative itself, which keeps the label attached to the type
 * wherever it is used, not just inside one variant.
 */

namespace dpack {

class VariantError : public std::runtime_error {
public:
  VariantError(const std::string& message) : std::runtime_error(message) {}
};

namespace detail {

// ==================================================================
// Query whether a type is a variant

template <typename T>
struct is_variant : std::false_type {};

template <typename... Args>
struct is_variant<std::variant<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_variant_v = is_variant<T>::value;

template <typename T>
concept variant_c = is_variant_v<std::remove_cvref_t<T>>;

// ==================================================================
// Build the variant label list

template <variant_c V, size_t... Is>
constexpr std::array<std::string_view, sizeof...(Is)> get_variant_labels(
    std::index_sequence<Is...>) {
  return {type_label<std::variant_alternative_t<Is, V>>()...};
}

// NOTE: Put the variant labels result in a struct in order to static_assert unique labels
// Since the user may define custom type label values, this isn't guaranteed
template <variant_c V>
struct VariantLabels {
  static constexpr std::array<std::string_view, std::variant_size_v<V>> value =
      get_variant_labels<V>(std::make_index_sequence<std::variant_size_v<V>>());
  static_assert(labels_are_unique(value), "Variant has duplicate labels");
};

// Fold over the alternatives, stopping at the first label that matches
// Constructs the alternative in-place, so only requires it to be default constructible
template <variant_c V, size_t... Is>
std::optional<V> get_variant_from_label(std::string_view label, std::index_sequence<Is...>) {
  std::optional<V> result;
  ((VariantLabels<V>::value[Is] == label ? (result.emplace(std::in_place_index<Is>), true)
                                         : false) ||
   ...);
  return result;
}

} // namespace detail

// ==================================================================
// "Public" functions

template <detail::variant_c V>
inline constexpr size_t variant_size = std::variant_size_v<V>;

// NOTE: Need to use "inline constexpr" if the address of variant_labels needs to taken (which it
// does) This forces the data to be stored in memory somewhere, which without inline would violate
// ODR since the symbol has external linkage and may be stored in multiple translation units

template <detail::variant_c V>
inline constexpr std::array<std::string_view, variant_size<V>> variant_labels =
    detail::VariantLabels<V>::value;

template <detail::variant_c V>
inline size_t variant_index(const V& value) {
  if (value.valueless_by_exception()) {
    throw VariantError("Variant is valueless by exception, so has no label");
  }
  return value.index();
}

template <detail::variant_c V>
inline std::optional<V> variant_from_label(std::string_view label) {
  return detail::get_variant_from_label<V>(label, std::make_index_sequence<variant_size<V>>());
}

template <detail::variant_c V>
inline std::string_view variant_to_label(const V& value) {
  return variant_labels<V>[variant_index(value)];
}

} // namespace dpack
