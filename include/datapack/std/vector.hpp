#pragma once

#include "datapack/datapack.hpp"
#include <vector>

namespace datapack {

template <typename T>
requires writeable<T>
void pack(const std::vector<T>& value, Writer& writer) {
  writer.list_begin();
  for (const auto& element : value) {
    writer.list_next();
    writer.value(element);
  }
  writer.list_end();
}

template <typename T>
requires readable<T>
void pack(std::vector<T>& value, Reader& reader) {
  value.clear();
  reader.list_begin();
  while (reader.list_next()) {
    value.emplace_back();
    reader.value(value.back());
  }
  reader.list_end();
}

template <typename T>
requires std::is_trivially_constructible_v<T>
void pack_trivial(const std::vector<T>& value, Writer& writer) {
  writer.binary(std::span<const std::uint8_t>(
      (const std::uint8_t*)value.data(), //
      value.size() * sizeof(T)));
}

template <typename T>
requires std::is_trivially_constructible_v<T>
void pack_trivial(std::vector<T>& value, Reader& reader) {
  auto bytes = reader.binary();
  if (bytes.size() % sizeof(T) != 0) {
    reader.invalidate();
    return;
  }
  value.resize(bytes.size() / sizeof(T));
  memcpy(value.data(), bytes.data(), bytes.size());
}

} // namespace datapack
