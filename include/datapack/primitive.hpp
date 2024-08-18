#pragma once

namespace datapack {

// Note: Use non-class enum to avoid needing Primitive prefix
enum Primitive {
    I32,
    I64,
    U32,
    U64,
    F32,
    F64,
    U8,
    BOOL,
    STRING
};

} // namespace datapack
