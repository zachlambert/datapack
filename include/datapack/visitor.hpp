#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include "datapack/definer.hpp"

namespace datapack {

class Visitor: public Readable, public Writeable, public Defined {};

} // namespace datapack

#define DATAPACK_VISITOR_FUNCS_DEF(T) \
template <typename V> \
void visit(V& visitor, T& value); \
inline void read(datapack::Reader& reader, T& value) { \
    visit(reader, value); \
} \
inline void write(datapack::Writer& writer, const T& value) { \
    visit(writer, const_cast<T&>(value)); \
} \
inline void define(datapack::Definer& definer, const T& value) { \
    visit(definer, const_cast<T&>(value)); \
}

#define DATAPACK_VISITOR_FUNCS_IMPL(T) \
template void visit<datapack::Reader>(datapack::Reader&, T&); \
template void visit<datapack::Writer>(datapack::Writer&, T&); \
template void visit<datapack::Definer>(datapack::Definer&, T&); \
static_assert(datapack::readable<T>); \
static_assert(datapack::writeable<T>); \
static_assert(datapack::defined<T>);

#define DATAPACK_VISITOR_METHODS_DEF(T) \
template <typename V> \
void visit(V& visitor); \
void read(datapack::Reader& reader) override { \
    visit(reader); \
} \
void write(datapack::Writer& writer) const override { \
    const_cast<T*>(this)->visit(writer); \
} \
void define(datapack::Definer& definer) const override { \
    const_cast<T*>(this)->visit(definer); \
}

#define DATAPACK_VISITOR_METHODS_IMPL(T) \
template void T::visit(datapack::Reader&); \
template void T::visit(datapack::Writer&); \
template void T::visit(datapack::Definer&); \
static_assert(datapack::readable<T>); \
static_assert(datapack::writeable<T>); \
static_assert(datapack::defined<T>);
