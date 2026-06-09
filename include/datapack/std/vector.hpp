#pragma once

#include "datapack/datapack.hpp"
#include <vector>

namespace datapack {

template <typename T>
requires writeable<T>
void write(Writer& writer, const std::vector<T>& value) {
  writer.list_begin(value.size());
  for (const auto& element : value) {
    writer.list_next();
    writer.value(element);
  }
  writer.list_end();
}

template <typename T>
requires readable<T>
void read(Reader& reader, std::vector<T>& value) {
  value.resize(reader.list_begin());
  for (size_t i = 0; i < value.size(); i++) {
    reader.value(value[i]);
  }
  reader.list_end();
}

} // namespace datapack
