#pragma once

#include <variant>
#include <vector>
#include <concepts>

namespace datapack {

template <typename T>
struct variant_details {};

template <typename T>
concept annotated_variant = requires(const char* label, T& value) {
    { variant_details<T>::labels } -> std::convertible_to<std::vector<const char*>>;
    { variant_details<T>::from_label(label) } -> std::convertible_to<T>;
    { variant_details<T>::to_label(value) } -> std::convertible_to<const char*>;
};

template <annotated_variant T>
const std::vector<const char*>& variant_labels() {
    return variant_details<T>::labels;
}

template <annotated_variant T>
T variant_from_label(const char* label) {
    return variant_details<T>::from_label(label);
}

template <annotated_variant T>
const char* variant_to_label(const T& value) {
    return variant_details<T>::to_label(value);
}

} // namespace datapack
