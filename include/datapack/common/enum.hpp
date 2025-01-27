#pragma once

#include "datapack/labelled_enum.hpp"
#include "datapack/packers.hpp"

namespace datapack {

template <labelled_enum T>
void pack(const T& value, Writer& writer) {
  writer.enumerate((int)value, enum_labels<T>[(int)value]);
}

template <labelled_enum T>
void pack(T& value, Reader& reader) {
  value = (T)reader.enumerate(enum_labels<T>);
}

} // namespace datapack
