#pragma once

#include <span>
#include <concepts>
#include <vector>


namespace datapack {

template <typename T>
struct enum_labels {};

template <typename T>
concept labelled_enum = requires(const char* label, T& value) {
    std::is_enum_v<T>;
    { enum_labels<T>::value } -> std::convertible_to<std::span<const char*>>;
};

template <labelled_enum T>
std::span<const char*> enum_labels_v() {
    return enum_labels<T>::value;
}

template <labelled_enum T>
bool enum_from_label(const char* label, T& value) {
    for (std::size_t i = 0; i < enum_labels<T>().size(); i++) {
        if (strcmp(label, enum_labels_v<T>()[i]) == 0) {
            value = (T)i;
            return true;
        }
    }
    return false;
}

template <labelled_enum T>
const char* enum_to_label(const T& value) {
    return enum_labels_v<T>()[(int)value];
}

} // namespace datapack

#define DATAPACK_LABELLED_ENUM(T) \
template <> \
struct enum_labels<Physics> { \
    static std::vector<const char*> value; \
};
