#pragma once

#include <array>
#include <cstring>
#include "datapack/datapack.hpp"


namespace datapack {

template <readable T, std::size_t Size>
void read(Reader& reader, std::array<T, Size>& value) {
    reader.list_begin();
    for (auto& element: value) {
        if (!reader.list_next()) {
            reader.error("Incorrect list length");
        }
        reader.value(element);
    }
    if (reader.list_next()) {
        reader.error("Incorrect list length");
    }
    reader.list_end();
}

template <writeable T, std::size_t Size>
void write(Writer& writer, const std::array<T, Size>& value) {
    writer.list_begin();
    for (const auto& element: value) {
        writer.list_next();
        writer.value(element);
    }
    writer.list_end();
}

template <typename T, std::size_t N>
requires std::is_trivially_copy_assignable_v<T>
void read_binary(Reader& reader, std::array<T, N>& value) {
    if constexpr(readable<T>) {
        if (reader.is_exhaustive()) {
            reader.binary_begin(sizeof(T));
            T dummy;
            reader.value(dummy);
            reader.binary_end();
            return;
        }
    }
    auto [data, size] = reader.binary_data();
    if (value.size() * sizeof(T) != size) {
        reader.error("Incorrect binary size");
    }
    std::memcpy(value.data(), data, size);
}

template <typename T, std::size_t N>
requires std::is_trivially_copy_assignable_v<T>
void write_binary(Writer& writer, const std::array<T, N>& value) {
    writer.binary_data((const std::uint8_t*)value.data(), value.size() * sizeof(T));
}

} // namespace datapack
