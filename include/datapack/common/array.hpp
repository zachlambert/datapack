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

template <defined T, std::size_t Size>
void define(Definer& definer, const std::array<T, Size>& value) {
    definer.tuple_begin();
    for (const auto& element: value) {
        definer.tuple_next();
        definer.value(element);
    }
    definer.tuple_end();
}

template <typename T, std::size_t N>
void read_binary(Reader& reader, std::array<T, N>& value, std::size_t expected_size) {
    if (value.size() != expected_size) {
        throw std::runtime_error("read_binary for array has incorrect expected_size");
    }
    static_assert(std::is_trivial_v<T>);
    std::size_t size = reader.binary_size();
    if (value.size() * sizeof(T) != size) {
        reader.error("Incorrect binary size");
    }
    reader.binary_data((std::uint8_t*)value.data());
}

template <typename T, std::size_t N>
void write_binary(Writer& writer, const std::array<T, N>& value) {
    static_assert(std::is_trivial_v<T>);
    writer.binary(value.size() * sizeof(T));
}

template <typename T, std::size_t N>
void define_binary(Definer& definer, const std::array<T, N>& value) {
    static_assert(std::is_trivial_v<T>);
    definer.binary(value.size() * sizeof(T));
    definer.value(T());
}

} // namespace datapack
