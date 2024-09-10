#pragma once

#include <cstring> // For memcpy
#include <vector>
#include "datapack/packers.hpp"


namespace datapack {

template <typename T>
requires writeable<T>
void pack(const std::vector<T>& value, Writer& writer) {
    if (std::is_trivially_constructible_v<T> && writer.trivial_as_binary()) {
        writer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), false);

    } else {
        writer.list_begin(std::is_trivially_constructible_v<T>);
        for (const auto& element: value) {
            writer.list_next();
            writer.value(element);
        }
        writer.list_end();
    }
}

template <typename T>
requires (std::is_trivially_constructible_v<T> && !writeable<T>)
void pack(const std::vector<T>& value, Writer& writer) {
    writer.binary_data((const std::uint8_t*)value.data(), value.size(), sizeof(T), false);
}

template <typename T>
requires readable<T>
void pack(std::vector<T>& value, Reader& reader) {
    if (std::is_trivially_constructible_v<T> && reader.trivial_as_binary()) {
        auto [data, length] = reader.binary_data(0, sizeof(T));
        value.resize(length);
        std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));

    } else {
        value.clear();
        reader.list_begin(std::is_trivially_constructible_v<T>);
        while (reader.list_next()) {
            value.emplace_back();
            reader.value(value.back());
        }
        reader.list_end();
    }
}

template <typename T>
requires (std::is_trivially_constructible_v<T> && !readable<T>)
void pack(std::vector<T>& value, Reader& reader) {
    auto [data, length] = reader.binary_data(0, sizeof(T));
    value.resize(length);
    std::memcpy((std::uint8_t*)value.data(), data, length * sizeof(T));
}

template <typename T>
requires editable<T>
void pack(std::vector<T>& value, Editor& editor) {
    if (editor.is_tokenizer()) {
        editor.list_begin();
        T temp;
        editor.value(temp);
        editor.list_end();
        return;
    }

    editor.list_begin();
    for (auto& element: value) {
        editor.list_next();
        editor.value(element);
    }
    switch (editor.list_end()) {
        case ContainerAction::Push:
            value.emplace_back();
            break;
        case ContainerAction::Pop:
            value.pop_back();
            break;
        default:
            break;
    }
}

} // namespace datapack
