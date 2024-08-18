#pragma once

#include <array>
#include <cstring>
#include "datapack/datapack.hpp"

namespace datapack {

template <typename T, std::size_t N>
requires (readable<T> || std::is_trivially_constructible_v<T>)
void read(Reader& reader, std::array<T, N>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_constructible_v<T> || !reader.trivial_as_binary()) {
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
    // Either !readable or (trivially_constructible_v && use_binary_arrays)
    auto [data, length] = reader.binary_data(N, sizeof(T));
    std::size_t size = length * sizeof(T);
    std::memcpy((std::uint8_t*)value.data(), data, size);
}

template <typename T, std::size_t N>
requires (writeable<T> || std::is_trivially_constructible_v<T>)
void write(Writer& writer, const std::array<T, N>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_constructible_v<T> || !writer.trivial_as_binary()) {
            std::size_t trivial_size = std::is_trivially_constructible_v<T> ? N * sizeof(T) : 0;
            writer.tuple_begin(trivial_size);
            for (const auto& element: value) {
                writer.tuple_next();
                writer.value(element);
            }
            writer.tuple_end(trivial_size);
            return;
        }
    }
    writer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), true);
}

} // namespace datapack
