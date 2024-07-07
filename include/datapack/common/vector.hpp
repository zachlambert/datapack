#pragma once

#include "datapack/datapack.hpp"
#include <cstring> // For memcpy
#include <vector>


namespace datapack {

template <typename T>
requires (readable<T> || std::is_trivially_copyable_v<T>)
void read(Reader& reader, std::vector<T>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_copyable_v<T> || !reader.use_binary_arrays()) {
            reader.list_begin(std::is_trivially_copyable_v<T>);
            value.clear();
            while (reader.list_next()) {
                value.emplace_back();
                reader.value(value.back());
            }
            reader.list_end();
            return;
        }
    }
    // Either !readable or (trivially_copyable_v && use_binary_arrays)
    auto [data, size] = reader.binary_data();
    if (size % sizeof(T) != 0) {
        reader.error("Invalid binary size");
        return;
    }
    value.resize(size / sizeof(T));
    std::memcpy((std::uint8_t*)value.data(), data, size);
}

template <typename T>
requires (writeable<T> || std::is_trivially_copyable_v<T>)
void write(Writer& writer, const std::vector<T>& value) {
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

template <typename T>
bool validate(const std::vector<T>& value, const LengthConstraint& constraint) {
    return value.size() == constraint.length;
}

} // namespace datapack
