#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"

#define DATAPACK(T) \
template <typename V> \
void visit(V& visitor, T& value); \
inline void read(::datapack::Reader& reader, T& value) { \
    visit(reader, value); \
} \
inline void write(::datapack::Writer& writer, const T& value) { \
    visit(writer, const_cast<T&>(value)); \
}

#define DATAPACK_IMPL(T) \
template void visit<::datapack::Reader>(::datapack::Reader&, T&); \
template void visit<::datapack::Writer>(::datapack::Writer&, T&); \
static_assert(::datapack::readable<T>); \
static_assert(::datapack::writeable<T>);

#define DATAPACK_METHODS(T) \
template <typename V> \
void visit(V& visitor); \
void read(::datapack::Reader& reader) { \
    visit(reader); \
} \
void write(::datapack::Writer& writer) const { \
    const_cast<T*>(this)->visit(writer); \
}

#define DATAPACK_METHODS_IMPL(T) \
template void T::visit(::datapack::Reader&); \
template void T::visit(::datapack::Writer&); \
static_assert(::datapack::readable<T>); \
static_assert(::datapack::writeable<T>);

#define DATAPACK_EMPTY(T) \
inline void read(::datapack::Reader& reader, T& value) { \
    reader.object_begin(); \
    reader.object_end(); \
} \
inline void write(::datapack::Writer& writer, const T& value) { \
    writer.object_begin(); \
    writer.object_end(); \
}
