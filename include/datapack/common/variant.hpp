#pragma once

#include "datapack/labelled_variant.hpp"
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include "datapack/schema.hpp"


namespace datapack {

template <labelled_variant T>
void read(Reader& reader, T& value) {
    const char* label = reader.variant_begin(variant_labels<T>());
    auto value_opt = variant_from_label<T>(label);
    if (!value_opt.has_value()) {
        throw ReadException("Invalid variant label '" + std::string(label) + "'");
    }
    value = value_opt.value();
    std::visit([&](auto& value) {
        reader.value(value);
    }, value);
    reader.variant_end();
}

template <labelled_variant T>
void write(Writer& writer, const T& value) {
    writer.variant_begin(variant_to_label(value), variant_labels<T>());
    std::visit([&](const auto& value){
        writer.value(value);
    }, value);
    writer.variant_end();
}

template <labelled_variant T>
void define(Definer& definer, const T& value) {
    definer.variant_begin();
    for (const auto& label: variant_labels<T>()) {
        definer.variant_next(label);
        std::visit([&](const auto& value){
            definer.value(value);
        }, variant_from_label<T>(label).value());
    }
    definer.variant_end();
}


} // namespace datapack
