#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include <cstring> // For memcpy
#include <vector>
#include <micro_types/vector.hpp>


namespace datapack {

#if 0
template <typename T, int Mode>
requires impl<T, Mode>
void pack(packref<std::vector<T>, Mode> value, Packer<Mode>& packer) {

}
#endif

template <typename T>
requires (readable<T> || std::is_trivially_constructible_v<T>)
void pack(Reader& reader, std::vector<T>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_constructible_v<T> || !reader.trivial_as_binary()) {
            reader.list_begin(std::is_trivially_constructible_v<T>);
            value.clear();
            while (reader.list_next()) {
                value.emplace_back();
                reader.value(value.back());
            }
            reader.list_end();
            return;
        }
    }
    // Either !readable or (trivially_constructible_v && use_binary_arrays)
    auto [data, length] = reader.binary_data(0, sizeof(T));
    value.resize(length);
    std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));
}

template <typename T>
requires (writeable<T> || std::is_trivially_constructible_v<T>)
void pack(Writer& writer, const std::vector<T>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_constructible_v<T> || !writer.trivial_as_binary()) {
            writer.list_begin(std::is_trivially_constructible_v<T>);
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

#if 0

template <typename T>
requires (readable<T> || std::is_trivially_constructible_v<T>)
void pack(Reader& reader, mct::vector<T>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_constructible_v<T> || !reader.trivial_as_binary()) {
            reader.list_begin(std::is_trivially_constructible_v<T>);
            value.clear();
            while (reader.list_next()) {
                value.emplace_back();
                reader.value(value.back());
            }
            reader.list_end();
            return;
        }
    }
    // Either !readable or (trivially_constructible_v && use_binary_arrays)
    auto [data, length] = reader.binary_data(0, sizeof(T));
    value.resize(length);
    std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));
}

template <typename T>
requires (writeable<T> || std::is_trivially_constructible_v<T>)
void pack(Writer& writer, const mct::vector<T>& value) {
    if constexpr(writeable<T>) {
        if (!std::is_trivially_constructible_v<T> || !writer.trivial_as_binary()) {
            writer.list_begin(std::is_trivially_constructible_v<T>);
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

#endif

} // namespace datapack
