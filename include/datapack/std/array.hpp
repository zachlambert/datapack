#pragma once

#include "datapack/datapack.hpp"
#include <array>
#include <cstring>

namespace datapack {

template <typename T, std::size_t N>
requires writeable<T>
void write(Writer& writer, const std::array<T, N>& value) {
  writer.tuple_begin();
  for (const auto& element : value) {
    writer.tuple_next();
    writer.value(element);
  }
  writer.tuple_end();
}

template <typename T, std::size_t N>
requires readable<T>
void read(Reader& reader, std::array<T, N>& value) {
  reader.tuple_begin();
  for (auto& element : value) {
    reader.tuple_next();
    reader.value(element);
  }
  reader.tuple_end();
  return;
}

} // namespace datapack
