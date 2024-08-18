#pragma once

#include "datapack/labelled_enum.hpp"
#include "datapack/reader.hpp"
#include "datapack/writer.hpp"


namespace datapack {

template <labelled_enum T>
void pack(T& value, Reader& packer) {
    value = (T)packer.enumerate(enum_labels<T>);
}

template <labelled_enum T>
void pack(const T& value, Writer& packer) {
    packer.enumerate((int)value, enum_labels<T>);
}

} // namespace datapack
