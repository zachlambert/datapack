#pragma once

#include <array>
#include <cstring>
#include "datapack/packers.hpp"

namespace datapack {

template <typename T, std::size_t N>
requires writeable<T>
void pack(const std::array<T, N>& value, Writer& writer) {
    if (std::is_trivially_constructible_v<T> && writer.trivial_as_binary()) {
        writer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), true);

    } else {
        std::size_t trivial_size = std::is_trivially_constructible_v<T> ? N * sizeof(T) : 0;
        writer.tuple_begin(trivial_size);
        for (const auto& element: value) {
            writer.tuple_next();
            writer.value(element);
        }
        writer.tuple_end(trivial_size);
    }
}

template <typename T, std::size_t N>
requires (!writeable<T> && std::is_trivially_constructible_v<T>)
void pack(const std::array<T, N>& value, Writer& writer) {
    writer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), true);
}

template <typename T, std::size_t N>
requires readable<T>
void pack(std::array<T, N>& value, Reader& reader) {
    if (std::is_trivially_constructible_v<T> && reader.trivial_as_binary()) {
        auto [data, length] = reader.binary_data(N, sizeof(T));
        std::size_t size = length * sizeof(T);
        std::memcpy((std::uint8_t*)value.data(), data, size);

    } else {
        std::size_t trivial_size = std::is_trivially_constructible_v<T> ? N * sizeof(T) : 0;
        reader.tuple_begin(trivial_size);
        for (auto& element: value) {
            reader.tuple_next();
            reader.value(element);
        }
        reader.tuple_end(trivial_size);
        return;
    }
}

template <typename T, std::size_t N>
requires (!readable<T> && std::is_trivially_constructible_v<T>)
void pack(std::array<T, N>& value, Reader& reader) {
    auto [data, length] = reader.binary_data(N, sizeof(T));
    std::size_t size = length * sizeof(T);
    std::memcpy((std::uint8_t*)value.data(), data, size);
}

template <typename T, std::size_t N>
requires editable<T>
void pack(std::array<T, N>& value, Editor& editor) {
    editor.tuple_begin();
    for (auto& element: value) {
        editor.tuple_next();
        editor.value(element);
    }
    editor.tuple_end();
    return;
}

} // namespace datapack
