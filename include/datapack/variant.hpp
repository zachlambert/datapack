#pragma once

#include <variant>
#include <vector>
#include <string>
#include <concepts>
#include <type_traits>

namespace datapack {

template <typename T>
struct variant_details {};

template <typename T>
concept annotated_variant = requires(const std::string& label, T& value) {
    { variant_details<T>::labels } -> std::convertible_to<std::vector<std::string>>;
    { variant_details<T>::from_label(label) } -> std::convertible_to<T>;
    { variant_details<T>::to_label(value) } -> std::convertible_to<std::string>;
};

template <annotated_variant T>
const std::vector<std::string>& variant_labels() {
    return variant_details<T>::labels;
}

template <annotated_variant T>
T variant_from_label(const std::string& label) {
    return variant_details<T>::from_label(label);
}

template <annotated_variant T>
std::string variant_to_label(const T& value) {
    return variant_details<T>::to_label(value);
}

} // namespace datapack
