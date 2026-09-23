#pragma once

#include "datapack/datapack.hpp"

namespace dpack {

namespace detail {

template <typename T>
struct VariantLabels;

template <typename... Args>
struct VariantLabels<std::variant<Args...>> {
  static constexpr std::array<std::string_view, sizeof...(Args)> value = {label_of<Args>()...};
  static_assert(
      labels_are_unique(value),
      "Variant has duplicate labels, two types must have the same label");
};

template <typename T>
struct is_variant : std::false_type {};

template <typename... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type {};

template <typename T>
inline constexpr bool is_variant_v = is_variant<T>::value;

template <typename T>
concept variant_c = is_variant_v<std::remove_cvref_t<T>>;

} // namespace detail

template <detail::variant_c T>
inline constexpr auto variant_labels = detail::VariantLabels<T>::value;

namespace detail {

template <variant_c T>
void read_variant_next(Reader& reader, T& value, int value_index, int index) {
  if (!reader.is_tokenizer()) {
    reader.invalidate();
  }
}

template <variant_c T, typename Next, typename... Args>
void read_variant_next(Reader& reader, T& value, int value_index, int index) {
  if (reader.is_tokenizer()) {
    Next dummy;
    reader.variant_tokenize(index);
    reader.value(dummy);
  } else if (value_index == index) {
    Next next;
    reader.value(next);
    value = next;
    return;
  }
  read_variant_next<T, Args...>(reader, value, value_index, index + 1);
}

} // namespace detail

template <detail::variant_c T>
void write(Writer& writer, const T& value) {
  writer.variant_begin(value.index(), variant_labels<T>);
  std::visit([&](const auto& value) { writer.value(value); }, value);
  writer.variant_end();
}

template <typename... Args>
void read(Reader& reader, std::variant<Args...>& value) {
  using T = std::variant<Args...>;
  int value_int = reader.variant_begin(variant_labels<T>);
  detail::read_variant_next<T, Args...>(reader, value, value_int, 0);
  reader.variant_end();
}

} // namespace dpack
