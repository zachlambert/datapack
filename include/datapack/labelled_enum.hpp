#pragma once

#include <vector>
#include <optional>
#include <concepts>


namespace datapack {

template <typename T>
struct enum_details {};

template <typename T>
concept labelled_enum = requires(const char* label, T& value) {
    std::is_enum_v<T>;
    { enum_details<T>::labels } -> std::convertible_to<std::vector<const char*>>;
    { enum_details<T>::from_label(label) } -> std::convertible_to<std::optional<T>>;
    { enum_details<T>::to_label(value) } -> std::convertible_to<const char*>;
};

template <labelled_enum T>
const std::vector<const char*>& enum_labels() {
    return enum_details<T>::labels;
}

template <labelled_enum T>
std::optional<T> enum_from_label(const char* label) {
    return enum_details<T>::from_label(label);
}

template <labelled_enum T>
const char* enum_to_label(const T& value) {
    return enum_details<T>::to_label(value);
}

#define DATAPACK_LABELLED_ENUM(T) \
template <> \
struct datapack::enum_details<Physics> { \
    static std::vector<const char*> labels; \
    static const char* to_label(const T& value); \
    static std::optional<T> from_label(const char* label); \
}; \
static_assert(datapack::labelled_enum<T>);

} // namespace datapack
