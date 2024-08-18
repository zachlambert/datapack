#pragma once

#include <concepts>

namespace datapack {

template <int Mode>
class Packer;

static constexpr int MODE_READ = 0;
static constexpr int MODE_WRITE = 1;
using Reader = Packer<MODE_READ>;
using Writer = Packer<MODE_WRITE>;

template <typename T, int Mode>
using packref = std::conditional_t<Mode==MODE_READ, T&, const T&>;

template <typename T, int Mode>
concept impl = requires(packref<T, Mode> type, Packer<Mode>& packer) {
    { pack(type, packer) };
};

#define DATAPACK(T) \
template <int Mode> \
void pack(packref<T, Mode>, Packer<Mode>&)

#define DATAPACK_FRIEND(T) \
template <int Mode> \
friend void pack(packref<T, Mode>, Packer<Mode>&)

#define DATAPACK_INLINE(T, VALUE_NAME, PACKER_NAME) \
template <int Mode> \
void pack(packref<T, Mode> VALUE_NAME, Packer<Mode>& PACKER_NAME)

#define DATAPACK_IMPL(T, value_name, packer_name) \
template void pack<MODE_READ>(T&, Packer<MODE_READ>&); \
template void pack<MODE_WRITE>(const T&, Packer<MODE_WRITE>&); \
template <int Mode> \
void pack(packref<T, Mode> value_name, Packer<Mode>& packer_name)

} // namespace datapack

#if 0

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"

namespace datapack {

template <typename V>
concept is_visitor = std::is_same_v<V, datapack::Reader> || std::is_same_v<V, datapack::Writer>;

} // namespace datapack

#define DATAPACK(T) \
template <::datapack::is_visitor V> \
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
template <::datapack::is_visitor V> \
void visit(V& visitor, T& value)

#define DATAPACK_INLINE(T) \
inline void read(::datapack::Reader& reader, T& value) { \
    visit(reader, value); \
} \
inline void write(::datapack::Writer& writer, T& value) { \
    visit(writer, const_cast<T&>(value)); \
} \
template <::datapack::is_visitor V> \
void visit(V& reader, T& value)

#define DATAPACK_METHODS(T) \
template <::datapack::is_visitor V> \
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
template <::datapack::is_visitor V> \
void T::visit(V& visitor) \

#define DATAPACK_EMPTY(T) \
inline void read(::datapack::Reader& reader, T& value) { \
    reader.object_begin(); \
    reader.object_end(); \
} \
inline void write(::datapack::Writer& writer, const T& value) { \
    writer.object_begin(); \
    writer.object_end(); \
}
#endif
