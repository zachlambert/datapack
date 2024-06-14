#pragma once

#include <array>
#include <cstring>
#include "datapack/datapack.hpp"


namespace datapack {

template <typename T, std::size_t N>
requires (readable<T> || std::is_trivially_copyable_v<T>)
void read(Reader& reader, std::array<T, N>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_copyable_v<T> || !reader.use_binary_arrays()) {
            reader.list_begin(std::is_trivially_copyable_v<T>);
            for (auto& element: value) {
                if (!reader.list_next()) {
                    break;
                }
                reader.value(element);
            }
            reader.list_end();
            return;
        }
    }
    // Either !readable or (trivially_copyable_v && use_binary_arrays)
    auto [data, size] = reader.binary_data();
    if (size != N * sizeof(T)) {
        reader.error("Invalid binary size");
        return;
    }
    std::memcpy((std::uint8_t*)value.data(), data, size);
}

template <typename T, std::size_t N>
requires (writeable<T> || std::is_trivially_copyable_v<T>)
void write(Writer& writer, const std::array<T, N>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_copyable_v<T> || !writer.use_binary_arrays()) {
            writer.list_begin(std::is_trivially_copyable_v<T>);
            for (const auto& element: value) {
                writer.list_next();
                writer.value(element);
            }
            writer.list_end();
            return;
        }
    }
    writer.binary_data((const std::uint8_t*)value.data(), value.size() * sizeof(T));
}

} // namespace datapack
