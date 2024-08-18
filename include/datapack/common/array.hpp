#pragma once

#include <array>
#include <cstring>
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"

namespace datapack {

template <typename T, std::size_t N>
requires (readable<T> || std::is_trivially_constructible_v<T>)
void pack(std::array<T, N>& value, Reader& packer) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_constructible_v<T> || !packer.trivial_as_binary()) {
            std::size_t trivial_size = std::is_trivially_constructible_v<T> ? N * sizeof(T) : 0;
            packer.tuple_begin(trivial_size);
            for (auto& element: value) {
                packer.tuple_next();
                packer.value(element);
            }
            packer.tuple_end(trivial_size);
            return;
        }
    }
    // Either !readable or (trivially_constructible_v && use_binary_arrays)
    auto [data, length] = packer.binary_data(N, sizeof(T));
    std::size_t size = length * sizeof(T);
    std::memcpy((std::uint8_t*)value.data(), data, size);
}

template <typename T, std::size_t N>
requires (writeable<T> || std::is_trivially_constructible_v<T>)
void write(const std::array<T, N>& value, Writer& packer) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_constructible_v<T> || !packer.trivial_as_binary()) {
            std::size_t trivial_size = std::is_trivially_constructible_v<T> ? N * sizeof(T) : 0;
            packer.tuple_begin(trivial_size);
            for (const auto& element: value) {
                packer.tuple_next();
                packer.value(element);
            }
            packer.tuple_end(trivial_size);
            return;
        }
    }
    packer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), true);
}

} // namespace datapack
