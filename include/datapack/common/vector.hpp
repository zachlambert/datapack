#pragma once

#include <vector>
#include <cstring>
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

template <typename T>
requires std::is_trivially_copy_assignable_v<T>
void read_binary(Reader& reader, std::vector<T>& value) {
    if constexpr(readable<T>) {
        if (reader.is_exhaustive()) {
            std::size_t size = reader.binary_begin();
            if (size % sizeof(T) != 0) {
                reader.error("Invalid binary size");
            }
            value.resize(size / sizeof(T));
            for (auto& element: value) {
                reader.value(element);
            }
            reader.binary_end();
            return;
        }
    }
    auto [data, size] = reader.binary_data();
    if (size % sizeof(T) != 0) {
        reader.error("Invalid binary size");
    }
    value.resize(size / sizeof(T));
    std::memcpy(value.data(), data, size);
}

template <typename T>
requires std::is_trivially_copy_assignable_v<T>
void write_binary(Writer& writer, const std::vector<T>& value) {
    writer.binary_data((const std::uint8_t*)value.data(), value.size() * sizeof(T));
}

template <typename T>
bool validate(const std::vector<T>& value, const LengthConstraint& constraint) {
    return value.size() == constraint.length;
}

} // namespace datapack
