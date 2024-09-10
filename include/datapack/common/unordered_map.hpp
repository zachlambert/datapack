#pragma once

#include <unordered_map>
#include "datapack/packers.hpp"


namespace datapack {

template <writeable K, writeable V>
void pack(const std::unordered_map<K, V>& value, Writer& writer) {
    writer.map_begin();
    for (const auto& pair: value) {
        writer.map_key();
        writer.value(pair.first);
        writer.map_value();
        writer.value(pair.second);
    }
    writer.map_end();
}

template <readable K, readable V>
void pack(std::unordered_map<K, V>& value, Reader& reader) {
    std::pair<K, V> pair;
    value.clear();
    reader.map_begin();
    auto iter = value.begin();
    while (reader.map_key()) {
        reader.value(pair.first);
        reader.map_value();
        reader.value(pair.second);
    }
    reader.map_end();
}

template <readable K, readable V>
void pack(std::unordered_map<K, V>& value, Editor& editor) {
    std::pair<K, V> pair;
    ListAction action;

    editor.map_begin();
    for (auto& [key, value]: value) {
        editor.map_key();
        editor.value(key);
        editor.map_value();
        editor.value(value);
    }
    if constexpr(std::is_default_constructible_v<V>) {
        K key;
        editor.map_insert_begin();
        editor.value(key);
        if (editor.map_insert_end()) {
            value.emplace(key, V());
        }
    }
    editor.map_end();
}

} // namespace datapack
