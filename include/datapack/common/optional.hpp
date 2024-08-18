#pragma once

#include "datapack/datapack.hpp"
#include <optional>
#include <micro_types/optional.hpp>


namespace datapack {

template <readable T>
void read(Reader& reader, std::optional<T>& value) {
    if (reader.optional_begin()) {
        value.emplace();
        reader.value(value.value());
        reader.optional_end();
    } else {
        value = std::nullopt;
    }
}

template <writeable T>
void write(Writer& writer, const std::optional<T>& value) {
    writer.optional_begin(value.has_value());
    if (value.has_value()) {
        writer.value(value.value());
        writer.optional_end();
    }
}

template <readable T>
void read(Reader& reader, mct::optional<T>& value) {
    if (reader.optional_begin()) {
        value.emplace();
        reader.value(value.value());
        reader.optional_end();
    } else {
        value = std::nullopt;
    }
}

template <writeable T>
void write(Writer& writer, const mct::optional<T>& value) {
    writer.optional_begin(value.has_value());
    if (value.has_value()) {
        writer.value(value.value());
        writer.optional_end();
    }
}

} // namespace datapack
