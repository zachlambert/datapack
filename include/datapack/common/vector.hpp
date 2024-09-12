#pragma once

#include <cstring> // For memcpy
#include <vector>
#include "datapack/packers.hpp"


namespace datapack {

template <typename T>
requires writeable<T>
void pack(const std::vector<T>& value, Writer& writer) {
    if (std::is_trivially_constructible_v<T> && writer.trivial_as_binary()) {
        writer.binary((const std::uint8_t*)value.data(), value.size(), sizeof(T), false);

    } else {
        writer.list_begin(std::is_trivially_constructible_v<T>);
        for (const auto& element: value) {
            writer.list_next();
            writer.value(element);
        }
        writer.list_end();
    }
}

template <typename T>
requires (std::is_trivially_constructible_v<T> && !writeable<T>)
void pack(const std::vector<T>& value, Writer& writer) {
    writer.binary((const std::uint8_t*)value.data(), value.size(), sizeof(T), false);
}

template <typename T>
requires readable<T>
void pack(std::vector<T>& value, Reader& reader) {
    if (std::is_trivially_constructible_v<T> && reader.trivial_as_binary()) {
        auto [data, length] = reader.binary(0, sizeof(T));
        value.resize(length);
        std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));

    } else {
        value.clear();
        reader.list_begin(std::is_trivially_constructible_v<T>);
        while (reader.list_next()) {
            value.emplace_back();
            reader.value(value.back());
        }
        reader.list_end();
    }
}

template <typename T>
requires (std::is_trivially_constructible_v<T> && !readable<T>)
void pack(std::vector<T>& value, Reader& reader) {
    auto [data, length] = reader.binary(0, sizeof(T));
    value.resize(length);
    std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));
}

} // namespace datapack
