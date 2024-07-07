#pragma once

#include <concepts>
#include <variant>
#include <span>
#include <micro_types/variant.hpp>


namespace datapack {

template <typename T>
struct variant_labels {};

template <typename T>
concept labelled_variant = requires() {
    { variant_labels<T>::value } -> std::convertible_to<std::span<const char*>>;
};

template <labelled_variant T>
std::span<const char*> variant_labels_v() {
    return variant_labels<T>::value;
}

template <labelled_variant T>
bool variant_from_label_iter(const char* label, std::size_t index, T& value) {
    return false;
}

template <labelled_variant T, typename Next, typename ...Remainder>
bool variant_from_label_iter(const char* label, std::size_t index, T& value) {
    if (strcmp(variant_labels_v<T>()[index], label) == 0) {
        value = Next();
        return true;
    }
    return variant_from_label_iter<T, Remainder...>(label, index+1, value);
}

template <typename ...Args>
requires labelled_variant<std::variant<Args...>>
bool variant_from_label(const char* label, std::variant<Args...>& value) {
    using T = std::variant<Args...>;
    return variant_from_label_iter<T, Args...>(label, 0, value);
}

template <typename ...Args>
requires labelled_variant<std::variant<Args...>>
bool variant_from_label(const char* label, mct::variant<Args...>& value) {
    using T = std::variant<Args...>;
    return variant_from_label_iter<T, Args...>(label, 0, value);
}

template <labelled_variant T>
const char* variant_to_label(const T& value) {
    return variant_labels_v<T>()[value.index()];
}

} // namespace datapack

#define DATAPACK_LABELLED_VARIANT(T) \
template <> \
struct variant_labels<T> { \
    static std::vector<const char*> value; \
};
