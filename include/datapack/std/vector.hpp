#pragma once

#include "datapack/datapack.hpp"
#include <vector>

namespace datapack {

template <typename T>
requires writeable<T>
void write(Writer& writer, const std::vector<T>& value) {
  writer.list_begin();
  for (const auto& element : value) {
    writer.list_next();
    writer.value(element);
  }
  writer.list_end();
}

template <typename T>
requires readable<T>
void read(Reader& reader, std::vector<T>& value) {
  value.clear();
  reader.list_begin();
  while (reader.list_next()) {
    value.emplace_back();
    reader.value(value.back());
  }
  reader.list_end();
}

} // namespace datapack
