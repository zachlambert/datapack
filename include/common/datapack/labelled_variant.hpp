#pragma once

#include <variant>
#include <vector>
#include <optional>
#include <concepts>


namespace datapack {

template <typename T>
struct variant_labels {};

template <typename T>
concept labelled_variant = requires() {
    { variant_labels<T>::value } -> std::convertible_to<std::vector<const char*>>;
};

template <labelled_variant T>
const std::vector<const char*>& variant_labels_v() {
    return variant_labels<T>::value;
}

template <labelled_variant T>
std::optional<T> variant_from_label_iter(const char* label, std::size_t index) {
    return std::nullopt;
}

template <labelled_variant T, typename Next, typename ...Remainder>
std::optional<T> variant_from_label_iter(const char* label, std::size_t index) {
    if (strcmp(variant_labels_v<T>()[index], label) == 0) {
        return Next();
    }
    return variant_from_label_iter<T, Remainder...>(label, index+1);
}

template <typename ...Args>
requires labelled_variant<std::variant<Args...>>
std::optional<std::variant<Args...>> variant_from_label(const char* label) {
    using T = std::variant<Args...>;
    return variant_from_label_iter<T, Args...>(label, 0);
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
