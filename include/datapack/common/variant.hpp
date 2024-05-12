#pragma once

#include "datapack/labelled_variant.hpp"
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"


namespace datapack {

template <labelled_variant T>
void read(Reader& reader, T& value) {
    const char* label = reader.variant(variant_labels<T>());
    value = variant_from_label<T>(label);
    std::visit([&](auto& value) {
        reader.value(value);
    }, value);
}

template <labelled_variant T>
void write(Writer& writer, const T& value) {
    writer.variant(variant_to_label(value), variant_labels<T>());
    std::visit([&](const auto& value){
        writer.value(value);
    }, value);
}

} // namespace datapack