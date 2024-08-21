#pragma once

#include <cstring> // For memcpy
#include <vector>
#include "datapack/packers.hpp"


namespace datapack {

template <typename T>
requires (readable<T> || std::is_trivially_constructible_v<T>)
void pack(std::vector<T>& value, Reader& reader) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_constructible_v<T> || !reader.trivial_as_binary()) {
            value.clear();
            reader.list_begin(std::is_trivially_constructible_v<T>);
            while (reader.list_next()) {
                value.emplace_back();
                reader.value(value.back());
            }
            reader.list_end();
            return;
        }
    }
    // Either !readable or (trivially_constructible_v && use_binary_arrays)
    auto [data, length] = reader.binary_data(0, sizeof(T));
    value.resize(length);
    std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));
}

template <typename T>
requires (writeable<T> || std::is_trivially_constructible_v<T>)
void pack(const std::vector<T>& value, Writer& writer) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_constructible_v<T> || !writer.trivial_as_binary()) {
            writer.list_begin(std::is_trivially_constructible_v<T>);
            for (const auto& element: value) {
                writer.list_next();
                writer.value(element);
            }
            writer.list_end();
            return;
        }
    }
    writer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), false);
}

} // namespace datapack
