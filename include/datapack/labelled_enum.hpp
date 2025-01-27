#pragma once

#include <concepts>
#include <span>
#include <vector>

namespace datapack {

template <typename T>
struct enum_details {};

template <typename T>
concept labelled_enum = requires(const char* label, T& value) {
  std::is_enum_v<T>;
  { enum_details<T>::size } -> std::convertible_to<std::size_t>;
  { enum_details<T>::labels } -> std::convertible_to<std::span<const char*>>;
};

template <labelled_enum T>
constexpr std::size_t enum_size = enum_details<T>::size;

template <labelled_enum T>
static const std::span<const char*> enum_labels = enum_details<T>::labels;

template <labelled_enum T>
bool enum_from_label(const char* label, T& value) {
  for (std::size_t i = 0; i < enum_labels<T>.size(); i++) {
    if (strcmp(label, enum_labels<T>[i]) == 0) {
      value = (T)i;
      return true;
    }
  }
  return false;
}

template <labelled_enum T>
const char* enum_to_label(const T& value) {
  return enum_labels<T>[(int)value];
}

} // namespace datapack

#define DATAPACK_LABELLED_ENUM(T, Size)                                                            \
  template <>                                                                                      \
  struct enum_details<T> {                                                                         \
    static constexpr std::size_t size = Size;                                                      \
    static const std::span<const char*> labels;                                                    \
    static const char* labels_array[Size];                                                         \
  }

#define DATAPACK_LABELLED_ENUM_DEF(T)                                                              \
  const std::span<const char*> enum_details<T>::labels =                                           \
      std::span(enum_details<T>::labels_array, enum_details<T>::size);                             \
  const char* enum_details<T>::labels_array[enum_size<T>]
