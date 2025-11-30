#pragma once

#include "datapack/datapack.hpp"
#include <unordered_map>

namespace datapack {

template <typename K, typename V>
requires writeable<K> && writeable<V>
void write(const std::unordered_map<K, V>& map, Writer& writer) {
  writer.list_begin();
  for (const auto& [key, value] : map) {
    writer.list_next();
    writer.tuple_begin();

    writer.tuple_next();
    writer.value(key);
    writer.tuple_next();
    writer.value(value);

    writer.tuple_end();
  }
  writer.list_end();
}

template <typename K, typename V>
requires readable<K> && readable<V>
void read(std::unordered_map<K, V>& map, Reader& reader) {
  map.clear();
  reader.list_begin();
  while (reader.list_next()) {
    reader.tuple_begin();

    K key;
    reader.tuple_next();
    reader.value(key);

    V value;
    reader.tuple_next();
    reader.value(value);

    map.emplace(key, value);

    reader.tuple_end();
  }
  reader.list_end();
}

} // namespace datapack
