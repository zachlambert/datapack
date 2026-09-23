#pragma once

#include "datapack/datapack.hpp"
#include "datapack/names/variant_names.hpp"

namespace dpack {

namespace detail {

template <variant_c V>
void read_variant_next(Reader& reader, V& value, int value_index, int index) {
  if (!reader.is_tokenizer()) {
    reader.invalidate();
  }
}

template <variant_c V, typename Next, typename... Args>
void read_variant_next(Reader& reader, V& value, int value_index, int index) {
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
  read_variant_next<V, Args...>(reader, value, value_index, index + 1);
}

} // namespace detail

template <detail::variant_c V>
void write(Writer& writer, const V& value) {
  writer.variant_begin((int)variant_index(value), variant_labels<V>);
  std::visit([&](const auto& value) { writer.value(value); }, value);
  writer.variant_end();
}

template <typename... Args>
void read(Reader& reader, std::variant<Args...>& value) {
  using V = std::variant<Args...>;
  int value_int = reader.variant_begin(variant_labels<V>);
  detail::read_variant_next<V, Args...>(reader, value, value_int, 0);
  reader.variant_end();
}

} // namespace dpack
