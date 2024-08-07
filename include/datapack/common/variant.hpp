#pragma once

#include "datapack/labelled_variant.hpp"
#include "datapack/datapack.hpp"
#include <micro_types/variant.hpp>


namespace datapack {

template <labelled_variant T>
void match_variant_next(Reader& reader, T& value, std::size_t index) {
    if (!reader.is_exhaustive()) {
        reader.set_error("No matching variant");
    }
}

template <labelled_variant T, typename Next, typename... Args>
void match_variant_next(Reader& reader, T& value, std::size_t index) {
    Next next;
    if (reader.variant_match(variant_labels<T>[index])) {
        reader.value(next);
        value = next;
        if (!reader.is_exhaustive()) {
            return;
        }
    }
    match_variant_next<T, Args...>(reader, value, index+1);
}

template <typename ...Args>
requires labelled_variant<std::variant<Args...>>
void read(Reader& reader, std::variant<Args...>& value) {
    using T = std::variant<Args...>;
    reader.variant_begin(variant_labels<T>);
    match_variant_next<T, Args...>(reader, value, 0);
    reader.variant_end();
}

template <labelled_variant T>
void write(Writer& writer, const T& value) {
    writer.variant_begin(variant_to_label(value), variant_labels<T>);
    std::visit([&](const auto& value){
        writer.value(value);
    }, value);
    writer.variant_end();
}

} // namespace datapack
