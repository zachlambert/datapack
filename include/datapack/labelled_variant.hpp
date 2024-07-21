#pragma once

#include <concepts>
#include <variant>
#include <span>
#include <micro_types/variant.hpp>


namespace datapack {

template <typename T>
struct variant_details {};

template <typename T>
concept labelled_variant = requires() {
    { variant_details<T>::size } -> std::convertible_to<std::size_t>;
    { variant_details<T>::labels } -> std::convertible_to<std::span<const char*>>;
};

template <labelled_variant T>
static constexpr std::size_t variant_size = variant_details<T>::size;

template <labelled_variant T>
static const std::span<const char*> variant_labels = variant_details<T>::labels;

template <labelled_variant T>
bool variant_from_label_iter(const char* label, std::size_t index, T& value) {
    return false;
}

template <labelled_variant T, typename Next, typename ...Remainder>
bool variant_from_label_iter(const char* label, std::size_t index, T& value) {
    if (strcmp(variant_labels<T>[index], label) == 0) {
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
    return variant_labels<T>[value.index()];
}

} // namespace datapack

#define DATAPACK_LABELLED_VARIANT(T, Size) \
template <> \
struct variant_details<T> { \
    static constexpr std::size_t size = Size; \
    static const std::span<const char*> labels; \
    static const char* labels_array[Size]; \
};

#define DATAPACK_LABELLED_VARIANT_DEF(T) \
const std::span<const char*> variant_details<T>::labels = std::span( \
    variant_details<T>::labels_array, \
    variant_details<T>::size); \
const char* variant_details<T>::labels_array[variant_size<T>]
