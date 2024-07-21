#pragma once

#include "datapack/labelled_enum.hpp"
#include "datapack/datapack.hpp"


namespace datapack {

template <labelled_enum T>
void read(Reader& reader, T& value) {
    value = (T)reader.enumerate(enum_labels<T>);
}

template <labelled_enum T>
void write(Writer& writer, const T& value) {
    writer.enumerate((int)value, enum_labels<T>);
}

} // namespace datapack
