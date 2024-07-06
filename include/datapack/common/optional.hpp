#pragma once

#include "datapack/datapack.hpp"
#include "datapack/types.hpp"


namespace datapack {

template <readable T>
void read(Reader& reader, optional_t<T>& value) {
    if (reader.optional_begin()) {
        value.emplace();
        reader.value(value.value());
        reader.optional_end();
    } else {
        value = std::nullopt;
    }
}

template <writeable T>
void write(Writer& writer, const optional_t<T>& value) {
    writer.optional_begin(value.has_value());
    if (value.has_value()) {
        writer.value(value.value());
        writer.optional_end();
    }
}

} // namespace datapack
