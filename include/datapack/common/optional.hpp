#pragma once

#include "datapack/packers.hpp"
#include <optional>


namespace datapack {

template <writeable T>
void pack(const std::optional<T>& value, Writer& writer) {
    writer.optional_begin(value.has_value());
    if (value.has_value()) {
        writer.value(value.value());
        writer.optional_end();
    }
}

template <readable T>
void pack(std::optional<T>& value, Reader& reader) {
    if (reader.optional_begin()) {
        value.emplace();
        reader.value(value.value());
        reader.optional_end();
    } else {
        value = std::nullopt;
    }
}

template <editable T>
void pack(std::optional<T>& value, Editor& editor) {
    if (editor.optional_begin(value.has_value())) {
        value.emplace();
        editor.value(value.value());
        editor.optional_end();
    } else {
        value = std::nullopt;
    }
}

} // namespace datapack
