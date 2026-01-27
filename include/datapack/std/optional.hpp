#pragma once

#include "datapack/datapack.hpp"
#include <optional>

namespace datapack {

template <typename T>
requires writeable<T>
void write(Writer& writer, const std::optional<T>& value) {
  writer.optional_begin(value.has_value());
  if (value.has_value()) {
    writer.value(value.value());
    writer.optional_end();
  }
}

template <typename T>
requires readable<T>
void read(Reader& reader, std::optional<T>& value) {
  if (reader.optional_begin()) {
    value.emplace();
    reader.value(value.value());
    reader.optional_end();
  } else {
    value = std::nullopt;
  }
}

} // namespace datapack
