#pragma once

#include "datapack/datapack.hpp"
#include <array>
#include <cstring>

namespace datapack {

template <typename T, std::size_t N>
requires writeable<T>
void pack(const std::array<T, N>& value, Writer& writer) {
  writer.tuple_begin(N);
  for (const auto& element : value) {
    writer.tuple_next();
    writer.value(element);
  }
  writer.tuple_end();
}

template <typename T, std::size_t N>
requires readable<T>
void pack(std::array<T, N>& value, Reader& reader) {
  reader.tuple_begin();
  for (auto& element : value) {
    reader.tuple_next();
    reader.value(element);
  }
  reader.tuple_end();
  return;
}

} // namespace datapack
