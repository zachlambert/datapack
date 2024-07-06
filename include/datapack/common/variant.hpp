#pragma once

#include "datapack/labelled_variant.hpp"
#include "datapack/datapack.hpp"
#include "datapack/types.hpp"


namespace datapack {

template <labelled_variant T>
void match_variant_next(Reader& reader, T& value, std::size_t index) {
    if (!reader.is_exhaustive()) {
        reader.error("No matching variant");
    }
}

template <labelled_variant T, typename Next, typename... Args>
void match_variant_next(Reader& reader, T& value, std::size_t index) {
    Next next;
    if (reader.variant_match(variant_labels_v<T>()[index])) {
        reader.value(next);
        value = next;
        if (!reader.is_exhaustive()) {
            return;
        }
    }
    match_variant_next<T, Args...>(reader, value, index+1);
}

template <typename ...Args>
requires labelled_variant<variant_t<Args...>>
void read(Reader& reader, variant_t<Args...>& value) {
    using T = variant_t<Args...>;
    reader.variant_begin(variant_labels_v<T>());
    match_variant_next<T, Args...>(reader, value, 0);
    reader.variant_end();
}

// TODO: Handle this for embedded case?
#ifndef EMBEDDED
template <labelled_variant T>
void write(Writer& writer, const T& value) {
    writer.variant_begin(variant_to_label(value), variant_labels_v<T>());
    std::visit([&](const auto& value){
        writer.value(value);
    }, value);
    writer.variant_end();
}
#endif

} // namespace datapack
