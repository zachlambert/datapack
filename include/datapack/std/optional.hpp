#pragma once

#include "datapack/datapack.hpp"
#include <optional>

namespace datapack {

template <writeable T>
void write(const std::optional<T>& value, Writer& writer) {
  writer.optional_begin(value.has_value());
  if (value.has_value()) {
    writer.value(value.value());
    writer.optional_end();
  }
}

template <readable T>
void read(std::optional<T>& value, Reader& reader) {
  if (reader.optional_begin()) {
    value.emplace();
    reader.value(value.value());
    reader.optional_end();
  } else {
    value = std::nullopt;
  }
}

} // namespace datapack
