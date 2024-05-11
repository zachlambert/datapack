#pragma once

#include <vector>
#include <string>
#include <concepts>

namespace datapack {

template <typename T>
struct enum_details {};

template <typename T>
concept annotated_enum = requires(const std::string& label, T& value) {
    std::is_enum_v<T>;
    { enum_details<T>::labels } -> std::convertible_to<std::vector<std::string>>;
    { enum_details<T>::from_label(label) } -> std::convertible_to<T>;
    { enum_details<T>::to_label(value) } -> std::convertible_to<std::string>;
};

template <annotated_enum T>
const std::vector<std::string>& enum_labels() {
    return enum_details<T>::labels;
}

template <annotated_enum T>
T enum_from_label(const std::string& label) {
    return enum_details<T>::from_label(label);
}

template <annotated_enum T>
std::string enum_to_label(const T& value) {
    return enum_details<T>::to_label(value);
}

} // namespace datapack
