#pragma once

#include "datapack/datapack.hpp"
#include <array>
#include <cstring>

namespace datapack {

template <typename T, std::size_t N>
requires writeable<T>
void pack(const std::array<T, N>& value, Writer& writer) {
  writer.tuple_begin();
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

template <typename T, std::size_t N>
requires std::is_trivially_constructible_v<T>
void pack_trivial(const std::array<T, N>& value, Writer& writer) {
  writer.binary(std::span<const std::uint8_t>(
      (const std::uint8_t*)value.data(), //
      N * sizeof(T)));
}

template <typename T, std::size_t N>
requires std::is_trivially_constructible_v<T>
void pack_trivial(std::array<T, N>& value, Reader& reader) {
  auto bytes = reader.binary();
  if (N * sizeof(T) != bytes.size()) {
    reader.invalidate();
    return;
  }
  memcpy(value.data(), bytes.data(), bytes.size());
}

} // namespace datapack
