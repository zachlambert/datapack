#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"

#define DATAPACK_VISITOR_FUNCS_DEF(T) \
template <typename V> \
void visit(V& visitor, T& value); \
inline void read(datapack::Reader& reader, T& value) { \
    visit(reader, value); \
} \
inline void write(datapack::Writer& writer, const T& value) { \
    visit(writer, const_cast<T&>(value)); \
}

#define DATAPACK_VISITOR_FUNCS_IMPL(T) \
template void visit<datapack::Reader>(datapack::Reader&, T&); \
template void visit<datapack::Writer>(datapack::Writer&, T&); \
static_assert(datapack::readable<T> && datapack::writeable<T>);

#define DATAPACK_VISITOR_METHODS_DEF() \
template <typename V> \
void visit(V& visitor); \
void read(datapack::Reader& reader) override { \
    visit(reader); \
} \
void write(datapack::Writer& writer) const override { \
    const_cast<Entity*>(this)->visit(writer); \
}

#define DATAPACK_VISITOR_METHODS_IMPL(T) \
template void T::visit(datapack::Reader&); \
template void T::visit(datapack::Writer&); \
static_assert(datapack::readable<T> && datapack::writeable<T>);
