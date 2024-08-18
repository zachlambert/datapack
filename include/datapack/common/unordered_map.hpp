#pragma once

#include <unordered_map>
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"


namespace datapack {

template <readable K, readable V>
void pack(std::unordered_map<K, V>& value, Reader& packer) {
    std::pair<K, V> pair;
    value.clear();
    packer.list_begin();
    while (packer.list_next()) {
        packer.tuple_begin();
        packer.tuple_next();
        packer.value(pair.first);
        packer.tuple_next();
        packer.value(pair.second);
        packer.tuple_end();
        value.insert(pair);
    }
    packer.list_end();
}

template <writeable K, writeable V>
void pack(const std::unordered_map<K, V>& value, Writer& packer) {
    packer.list_begin();
    for (const auto& pair: value) {
        packer.list_next();
        packer.tuple_begin();
        packer.tuple_next();
        packer.value(pair.first);
        packer.tuple_next();
        packer.value(pair.second);
        packer.tuple_end();
    }
    packer.list_end();
}

} // namespace datapack
