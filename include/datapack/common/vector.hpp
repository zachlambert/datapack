#pragma once

#include <vector>
#include "datapack/visitor.hpp"


namespace datapack {

template <readable T>
void read(Reader& reader, std::vector<T>& value) {
    value.clear();
    reader.list_begin();
    while (reader.list_next()) {
        value.emplace_back();
        reader.value(value.back());
    }
    reader.list_end();
}

template <writeable T>
void write(Writer& writer, const std::vector<T>& value) {
    writer.list_begin();
    for (const auto& element: value) {
        writer.list_next();
        writer.value(element);
    }
    writer.list_end();
}

template <defined T>
void define(Definer& definer, const std::vector<T>& value) {
    definer.list();
    definer.value(T());
}

template <typename T>
requires std::is_trivial_v<T>
void read_binary(Reader& reader, std::vector<T>& value) {
    std::size_t size = reader.binary_size();
    if (size % sizeof(T) != 0) {
        reader.error("Incorrect binary size: " + std::to_string(size) + ", T=" + std::to_string(sizeof(T)));
    }
    value.resize(size / sizeof(T));
    reader.binary_data((std::uint8_t*)value.data());
}

template <typename T>
requires std::is_trivial_v<T>
void write_binary(Writer& writer, const std::vector<T>& value) {
    writer.binary(value.size() * sizeof(T), (const std::uint8_t*)value.data());
}

template <typename T>
requires std::is_trivial_v<T>
void define_binary(Definer& definer, const std::vector<T>& value) {
    definer.binary();
    definer.value(T());
}

template <typename T>
bool validate(const std::vector<T>& value, const LengthConstraint& constraint) {
    return value.size() == constraint.length;
}

} // namespace datapack
