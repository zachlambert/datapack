#pragma once

#include <unordered_map>
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"


namespace datapack {

template <readable K, readable V>
void read(Reader& reader, std::unordered_map<K, V>& value) {
    if constexpr(std::is_same_v<K, std::string>) {
        std::string key;
        reader.map_begin();
        while (reader.map_next(key)) {
            V element;
            reader.value(element);
            value.emplace(key, element);
        }
        reader.map_end();
    }
    if constexpr(!std::is_same_v<K, std::string>) {
        std::pair<K, V> pair;
        reader.list_begin();
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
}

template <writeable K, writeable V>
void write(Writer& writer, const std::unordered_map<K, V>& value) {
    if constexpr(std::is_same_v<K, std::string>) {
        std::string key;
        writer.map_begin();
        for (const auto& pair: value) {
            writer.map_next(pair.first);
            writer.value(pair.second);
        }
        writer.map_end();
    }
    if constexpr(!std::is_same_v<K, std::string>) {
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
}

} // namespace datapack
