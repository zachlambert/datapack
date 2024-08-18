#pragma once

#include <concepts>

namespace datapack {

template <int Mode>
class Packer;

static constexpr int MODE_READ = 0;
static constexpr int MODE_WRITE = 1;
using Reader = Packer<MODE_READ>;
using Writer = Packer<MODE_WRITE>;

template <int Mode, typename T>
using packref = std::conditional_t<Mode==MODE_READ, T&, const T&>;

template <int Mode, typename T>
struct pack_impl {};

template <int Mode, typename T>
concept has_impl = requires(Packer<Mode>& packer, packref<Mode, T> value) {
    { pack_impl<Mode, T>::impl(packer, value) };
};

template <int Mode, typename T>
inline void pack(Packer<Mode>& packer, packref<Mode, T> value) {
    pack_impl<Mode, T>::impl(packer, value);
}

#define DATAPACK(T) \
template <int Mode> \
struct pack_impl<Mode, T> { \
    static void impl(Packer<Mode>&, packref<Mode, T>); \
};

#define DATAPACK_INLINE(T, packer_name, value_name) \
template <int Mode> \
struct pack_impl<Mode, T> { \
    static void impl(Packer<Mode>&, packref<Mode, T>); \
}; \
template <int Mode> \
void pack_impl<Mode, T>::impl(Packer<Mode>& packer_name, packref<Mode, T> value_name)

// #define DATAPACK_INLINE(T, VALUE_NAME, PACKER_NAME) \
// template <int Mode> \
// void pack(packref<T, Mode> VALUE_NAME, Packer<Mode>& PACKER_NAME)

#define DATAPACK_IMPL(T, packer_name, value_name) \
template struct pack_impl<MODE_READ, T>; \
template struct pack_impl<MODE_WRITE, T>; \
template <int Mode> \
void pack_impl<Mode, T>::impl(Packer<Mode>& packer_name, packref<Mode, T> value_name)

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
