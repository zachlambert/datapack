#pragma once

#include <optional>
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include "datapack/schema.hpp"


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
    writer.optional(value.has_value());
    if (value.has_value()) {
        writer.value(value.value());
    }
}

template <defined T>
void define(Definer& definer, const std::optional<T>& value) {
    definer.optional();
    definer.value(T());
}


} // namespace datapack
