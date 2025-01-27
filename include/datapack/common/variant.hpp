#pragma once

#include "datapack/labelled_variant.hpp"
#include "datapack/packers.hpp"

namespace datapack {

template <labelled_variant T> void pack(const T &value, Writer &writer) {
  writer.variant_begin(value.index(), variant_labels<T>[value.index()]);
  std::visit([&](const auto &value) { writer.value(value); }, value);
  writer.variant_end();
}

template <labelled_variant T>
void read_variant_next(Reader &reader, T &value, int value_index, int index) {
  if (!reader.is_tokenizer()) {
    reader.invalidate();
  }
}

template <labelled_variant T, typename Next, typename... Args>
void read_variant_next(Reader &reader, T &value, int value_index, int index) {
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

template <typename... Args>
  requires labelled_variant<std::variant<Args...>>
void pack(std::variant<Args...> &value, Reader &reader) {
  using T = std::variant<Args...>;
  int value_int = reader.variant_begin(variant_labels<T>);
  read_variant_next<T, Args...>(reader, value, value_int, 0);
  reader.variant_end();
}

} // namespace datapack
