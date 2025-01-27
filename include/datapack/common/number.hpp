#pragma once

#include "datapack/number.hpp"
#include "datapack/packers.hpp"
#include <cstdint>

namespace datapack {

DATAPACK_INLINE(std::int32_t, value, packer) {
  packer.integer(IntType::I32, &value);
}
DATAPACK_INLINE(std::int64_t, value, packer) {
  packer.integer(IntType::I64, &value);
}
DATAPACK_INLINE(std::uint32_t, value, packer) {
  packer.integer(IntType::U32, &value);
}
DATAPACK_INLINE(std::uint64_t, value, packer) {
  packer.integer(IntType::U64, &value);
}

DATAPACK_INLINE(std::uint8_t, value, packer) {
  packer.integer(IntType::U8, &value);
}

DATAPACK_INLINE(float, value, packer) {
  packer.floating(FloatType::F32, &value);
}

DATAPACK_INLINE(double, value, packer) {
  packer.floating(FloatType::F64, &value);
}

inline void pack(const bool &value, Writer &writer) { writer.boolean(value); }
inline void pack(bool &value, Reader &reader) { value = reader.boolean(); }

} // namespace datapack
