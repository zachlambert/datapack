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
    auto [data, length] = reader.binary_data(0, sizeof(T));
    value.resize(length);
    std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));
}

template <typename T>
requires (writeable<T> || std::is_trivially_copyable_v<T>)
void write(Writer& writer, const std::vector<T>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_copyable_v<T> || !writer.trivial_as_binary()) {
            writer.list_begin(std::is_trivially_copyable_v<T>);
            for (const auto& element: value) {
                writer.list_next();
                writer.value(element);
            }
            writer.list_end();
            return;
        }
    }
    writer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), false);
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
    auto [data, length] = reader.binary_data(0, sizeof(T));
    value.resize(length);
    std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));
}

template <typename T>
requires (writeable<T> || std::is_trivially_copyable_v<T>)
void write(Writer& writer, const mct::vector<T>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_copyable_v<T> || !writer.trivial_as_binary()) {
            writer.list_begin(std::is_trivially_copyable_v<T>);
            for (const auto& element: value) {
                writer.list_next();
                writer.value(element);
            }
            writer.list_end();
            return;
        }
    }
    writer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), false);
}

template <typename T>
bool validate(const mct::vector<T>& value, const LengthConstraint& constraint) {
    return value.size() == constraint.length;
}

} // namespace datapack
