#pragma once

#include "datapack/datapack.hpp"
#include <cstring> // For memcpy
#include <vector>
#include <micro_types/vector.hpp>


namespace datapack {

template <typename T>
requires (readable<T> || std::is_trivially_copyable_v<T>)
void read(Reader& reader, std::vector<T>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_copyable_v<T> || !reader.trivial_as_binary()) {
            if (std::is_trivially_copyable_v<T>) {
                reader.trivial_begin(1);
            }
            reader.list_begin();
            value.clear();
            while (reader.list_next()) {
                value.emplace_back();
                reader.value(value.back());
            }
            reader.list_end();
            if (std::is_trivially_copyable_v<T>) {
                reader.trivial_end(1);
            }
            return;
        }
    }
    // Either !readable or (trivially_copyable_v && use_binary_arrays)
    auto [data, size] = reader.binary_data();
    if (size % sizeof(T) != 0) {
        reader.set_error("Invalid binary size");
        return;
    }
    value.resize(size / sizeof(T));
    std::memcpy((std::uint8_t*)value.data(), data, size);
}

template <typename T>
requires (writeable<T> || std::is_trivially_copyable_v<T>)
void write(Writer& writer, const std::vector<T>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_copyable_v<T> || !writer.trivial_as_binary()) {
            if (std::is_trivially_copyable_v<T>) {
                writer.trivial_begin(1);
            }
            writer.list_begin();
            for (const auto& element: value) {
                writer.list_next();
                writer.value(element);
            }
            writer.list_end();
            if (std::is_trivially_copyable_v<T>) {
                writer.trivial_end(1);
            }
            return;
        }
    }
    writer.binary_data((const std::uint8_t*)value.data(), value.size() * sizeof(T));
}

template <typename T>
bool validate(const std::vector<T>& value, const LengthConstraint& constraint) {
    return value.size() == constraint.length;
}


template <typename T>
requires (readable<T> || std::is_trivially_copyable_v<T>)
void read(Reader& reader, mct::vector<T>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_copyable_v<T> || !reader.trivial_as_binary()) {
            if (std::is_trivially_copyable_v<T>) {
                reader.trivial_begin(1);
            }
            reader.list_begin();
            value.clear();
            while (reader.list_next()) {
                value.emplace_back();
                reader.value(value.back());
            }
            reader.list_end();
            if (std::is_trivially_copyable_v<T>) {
                reader.trivial_end(1);
            }
            return;
        }
    }
    // Either !readable or (trivially_copyable_v && use_binary_arrays)
    auto [data, size] = reader.binary_data();
    if (size % sizeof(T) != 0) {
        reader.set_error("Invalid binary size");
        return;
    }
    value.resize(size / sizeof(T));
    std::memcpy((std::uint8_t*)value.data(), data, size);
}

template <typename T>
requires (writeable<T> || std::is_trivially_copyable_v<T>)
void write(Writer& writer, const mct::vector<T>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_copyable_v<T> || !writer.trivial_as_binary()) {
            if (std::is_trivially_copyable_v<T>) {
                writer.trivial_begin(1);
            }
            writer.list_begin();
            for (const auto& element: value) {
                writer.list_next();
                writer.value(element);
            }
            writer.list_end();
            if (std::is_trivially_copyable_v<T>) {
                writer.trivial_end(1);
            }
            return;
        }
    }
    writer.binary_data((const std::uint8_t*)value.data(), value.size() * sizeof(T));
}

template <typename T>
bool validate(const mct::vector<T>& value, const LengthConstraint& constraint) {
    return value.size() == constraint.length;
}

} // namespace datapack
