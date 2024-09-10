#pragma once

#include "datapack/packers.hpp"
#include "datapack/labelled_variant.hpp"


namespace datapack {


template <labelled_variant T>
void pack(const T& value, Writer& writer) {
    writer.variant_begin(value.index(), variant_labels<T>[value.index()]);
    std::visit([&](const auto& value){
        writer.value(value);
    }, value);
    writer.variant_end();
}


template <labelled_variant T>
void read_variant_next(Reader& reader, T& value, int value_index, int index) {
    reader.invalidate();
}

template <labelled_variant T, typename Next, typename... Args>
void read_variant_next(Reader& reader, T& value, int value_index, int index) {
    if (value_index == index) {
        Next next;
        reader.value(next);
        value = next;
        return;
    }
    read_variant_next<T, Args...>(reader, value, value_index, index+1);
}

template <typename ...Args>
requires labelled_variant<std::variant<Args...>>
void pack(std::variant<Args...>& value, Reader& reader) {
    using T = std::variant<Args...>;
    int value_int = reader.variant_begin(variant_labels<T>);
    read_variant_next<T, Args...>(reader, value, value_int, 0);
    reader.variant_end();
}


template <labelled_variant T>
void edit_variant_next(Editor& editor, T& value, int value_index, int index) {
    // Do nothing
}

template <labelled_variant T, typename Next, typename... Args>
void edit_variant_next(Editor& editor, T& value, int value_index, int index) {
    if (editor.is_tokenizer()) {
        Next next;
        editor.variant_next(index);
        editor.value(next);

    } else if (value_index == index) {
        editor.variant_next(index);
        if (value.index() == value_index) {
            editor.value(value.value());
        } else {
            Next next;
            editor.value(next);
            value = next;
        }
        return;
    }
    edit_variant_next<T, Args...>(editor, value, value_index, index+1);
}

template <typename ...Args>
requires labelled_variant<std::variant<Args...>>
void pack(std::variant<Args...>& value, Editor& editor) {
    using T = std::variant<Args...>;
    int value_int = editor.variant_begin(value.index(), variant_labels<T>[value.index()]);
    read_variant_next<T, Args...>(editor, value, value_int, 0);
    editor.variant_end();
}

} // namespace datapack
