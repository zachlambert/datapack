#pragma once

#include "datapack/packers.hpp"
#include <cstdint>


namespace datapack {

DATAPACK_INLINE(std::int32_t, value, packer) {
    packer.primitive(Primitive::I32, &value);
}
DATAPACK_INLINE(std::int64_t, value, packer) {
    packer.primitive(Primitive::I64, &value);
}
DATAPACK_INLINE(std::uint32_t, value, packer) {
    packer.primitive(Primitive::U32, &value);
}
DATAPACK_INLINE(std::uint64_t, value, packer) {
    packer.primitive(Primitive::U64, &value);
}

DATAPACK_INLINE(float, value, packer) {
    packer.primitive(Primitive::F32, &value);
}
DATAPACK_INLINE(double, value, packer) {
    packer.primitive(Primitive::F64, &value);
}

DATAPACK_INLINE(std::uint8_t, value, packer) {
    packer.primitive(Primitive::U8, &value);
}

DATAPACK_INLINE(bool, value, packer) {
    packer.primitive(Primitive::BOOL, &value);
}

} // namespace datapack
