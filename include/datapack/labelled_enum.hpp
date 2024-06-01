#pragma once

#include <vector>
#include <optional>
#include <concepts>


namespace datapack {

template <typename T>
struct enum_labels {};

template <typename T>
concept labelled_enum = requires(const char* label, T& value) {
    std::is_enum_v<T>;
    { enum_labels<T>::value } -> std::convertible_to<std::vector<const char*>>;
};

template <labelled_enum T>
const std::vector<const char*>& enum_labels_v() {
    return enum_labels<T>::value;
}

template <labelled_enum T>
std::optional<T> enum_from_label(const char* label) {
    for (std::size_t i = 0; i < enum_labels<T>().size(); i++) {
        if (strcmp(label, enum_labels_v<T>()[i]) == 0) {
            return (T)i;
        }
    }
    return std::nullopt;
}

template <labelled_enum T>
const char* enum_to_label(const T& value) {
    return enum_labels_v<T>()[(int)value];
}

#define DATAPACK_LABELLED_ENUM(T) \
template <> \
struct datapack::enum_labels<Physics> { \
    static std::vector<const char*> value; \
}; \
static_assert(datapack::labelled_enum<T>);

} // namespace datapack
