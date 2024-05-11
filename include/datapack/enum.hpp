#pragma once

#include <vector>
#include <concepts>

namespace datapack {

template <typename T>
struct enum_details {};

template <typename T>
concept annotated_enum = requires(const char* label, T& value) {
    std::is_enum_v<T>;
    { enum_details<T>::labels } -> std::convertible_to<std::vector<const char*>>;
    { enum_details<T>::from_label(label) } -> std::convertible_to<T>;
    // { enum_details<T>::to_label(value) } -> std::convertible_to<const char*>;
};

template <annotated_enum T>
const std::vector<const char*>& enum_labels() {
    return enum_details<T>::labels;
}

template <annotated_enum T>
T enum_from_label(const char* label) {
    return enum_details<T>::from_label(label);
}

template <annotated_enum T>
const char* enum_to_label(const T& value) {
    return enum_details<T>::to_label(value);
}

} // namespace datapack
