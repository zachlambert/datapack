#pragma once

#include <optional>
#include "datapack/datapack.hpp"


namespace datapack {

template <readable T>
void read(Reader& reader, std::optional<T>& value) {
    if (reader.optional()) {
        value.emplace();
        reader.value(value.value());
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

} // namespace datapack
