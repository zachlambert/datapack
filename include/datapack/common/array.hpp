#pragma once

#include <array>
#include <cstring>
#include "datapack/datapack.hpp"

namespace datapack {

template <typename T, std::size_t N>
requires (readable<T> || std::is_trivially_copyable_v<T>)
void read(Reader& reader, std::array<T, N>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_copyable_v<T> || !reader.trivial_as_binary()) {
            if (std::is_trivially_copyable_v<T>) {
                reader.trivial_begin(1);
            }
            reader.tuple_begin();
            for (auto& element: value) {
                reader.tuple_next();
                reader.value(element);
            }
            reader.tuple_end();
            if (std::is_trivially_copyable_v<T>) {
                reader.trivial_end(1);
            }
            return;
        }
    }
    // Either !readable or (trivially_copyable_v && use_binary_arrays)
    auto [data, size] = reader.binary_data();
    if (size != N * sizeof(T)) {
        reader.set_error("Invalid binary size");
        return;
    }
    std::memcpy((std::uint8_t*)value.data(), data, size);
}

template <typename T, std::size_t N>
requires (writeable<T> || std::is_trivially_copyable_v<T>)
void write(Writer& writer, const std::array<T, N>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_copyable_v<T> || !writer.trivial_as_binary()) {
            if (std::is_trivially_copyable_v<T>) {
                writer.trivial_begin(1);
            }
            writer.tuple_begin();
            for (const auto& element: value) {
                writer.list_next();
                writer.value(element);
            }
            writer.tuple_end();
            if (std::is_trivially_copyable_v<T>) {
                writer.trivial_end(1);
            }
            return;
        }
    }
    writer.binary_data((const std::uint8_t*)value.data(), value.size() * sizeof(T));
}

} // namespace datapack
