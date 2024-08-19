#pragma once

#include "datapack/datapack.hpp"
#include <cstring> // For memcpy
#include <vector>
#include <micro_types/vector.hpp>


namespace datapack {

template <typename T>
requires (readable<T> || std::is_trivially_constructible_v<T>)
void read(Reader& reader, std::vector<T>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_constructible_v<T> || !reader.trivial_as_binary()) {
            reader.list_begin(std::is_trivially_constructible_v<T>);
            std::size_t i = 0;
            while (true) {
                auto result = reader.list_next(i < value.size());
                if (result == ListNext::End) {
                    break;
                }
                if (result == ListNext::Remove) {
                    value.erase(value.begin() + i);
                    continue;
                }
                if (result == ListNext::Insert) {
                    value.insert(value.begin() + i, T());
                }
                if (i >= value.size()) {
                    value.emplace_back();
                }
                reader.value(value[i]);
                i++;
            }
            reader.list_end();
            value.resize(i);
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
void write(Writer& writer, const std::vector<T>& value) {
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

template <typename T>
requires (readable<T> || std::is_trivially_constructible_v<T>)
void read(Reader& reader, mct::vector<T>& value) {
    if constexpr (readable<T>) {
        if (!std::is_trivially_constructible_v<T> || !reader.trivial_as_binary()) {
            reader.list_begin(std::is_trivially_constructible_v<T>);
            std::size_t i = 0;
            while (true) {
                auto result = reader.list_next(i < value.size());
                if (result == ListNext::End) {
                    break;
                }
                if (result == ListNext::Remove) {
                    value.erase(value.begin() + i);
                    continue;
                }
                if (i >= value.size()) {
                    value.emplace_back();
                }
                reader.value(value[i]);
                i++;
            }
            reader.list_end();
            value.resize(i);
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
void write(Writer& writer, const mct::vector<T>& value) {
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

} // namespace datapack
