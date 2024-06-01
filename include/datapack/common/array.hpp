#pragma once

#include <array>
#include "datapack/visitor.hpp"


namespace datapack {

template <readable T, std::size_t Size>
void read(Reader& reader, std::array<T, Size>& value) {
    reader.tuple_begin();
    for (auto& element: value) {
        reader.tuple_next();
        reader.value(element);
    }
    reader.tuple_end();
}

template <writeable T, std::size_t Size>
void write(Writer& writer, const std::array<T, Size>& value) {
    writer.tuple_begin();
    for (const auto& element: value) {
        writer.tuple_next();
        writer.value(element);
    }
    writer.tuple_end();
}

#if 0
template <defined T, std::size_t Size>
void define(Definer& definer, const std::array<T, Size>& value) {
    definer.tuple_begin();
    for (const auto& element: value) {
        definer.tuple_next();
        definer.value(element);
    }
    definer.tuple_end();
}
#endif

template <typename T, std::size_t N>
requires std::is_trivial_v<T>
void read_binary(Reader& reader, std::array<T, N>& value) {
    std::size_t size = reader.binary_size();
    if (value.size() * sizeof(T) != size) {
        reader.error("Incorrect binary size " + std::to_string(size) + ", vsize " + std::to_string(value.size()) + ", dsize " + std::to_string(sizeof(T)));
    }
    reader.binary_data((std::uint8_t*)value.data());
}

template <typename T, std::size_t N>
requires std::is_trivial_v<T>
void write_binary(Writer& writer, const std::array<T, N>& value) {
    writer.binary(value.size() * sizeof(T), (std::uint8_t*)value.data());
}

#if 0
template <typename T, std::size_t N>
requires std::is_trivial_v<T>
void define_binary(Definer& definer, const std::array<T, N>& value) {
    definer.binary(value.size() * sizeof(T));
    definer.value(T());
}
#endif

} // namespace datapack
