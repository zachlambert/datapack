#pragma once

#include <unordered_map>
#include "datapack/packers.hpp"


namespace datapack {

template <writeable K, writeable V>
void pack(const std::unordered_map<K, V>& value, Writer& writer) {
    writer.list_begin();
    for (const auto& pair: value) {
        writer.list_next();
        writer.tuple_begin();
        writer.tuple_next();
        writer.value(pair.first);
        writer.tuple_next();
        writer.value(pair.second);
        writer.tuple_end();
    }
    writer.list_end();
}

template <readable K, readable V>
void pack(std::unordered_map<K, V>& value, Reader& reader) {
    std::pair<K, V> pair;
    value.clear();
    reader.list_begin();
    auto iter = value.begin();
    while (reader.list_next()) {
        reader.tuple_begin();
        reader.tuple_next();
        reader.value(pair.first);
        reader.tuple_next();
        reader.value(pair.second);
        reader.tuple_end();
        value.insert(pair);
    }
    reader.list_end();
}

} // namespace datapack
