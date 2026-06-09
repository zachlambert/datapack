#pragma once

#include "datapack/hint.hpp"
#include "datapack/labelled_enum.hpp"
#include <concepts>
#include <cstdint>
#include <span>
#include <string>

namespace dpack {

enum class NumberType { I32, I64, U32, U64, U8, F32, F64 };

class Writer;

template <typename T>
concept writeable = requires(Writer& writer, const T& value) {
  { write(writer, value) };
};

template <typename T>
concept writeable_method = requires(const T& value, Writer& writer) {
  { value.write(writer) };
};
template <writeable_method T>
inline void write(Writer& writer, const T& value) {
  value.write(writer);
}

class Reader;

template <typename T>
concept readable = requires(Reader& reader, T& value) {
  { read(reader, value) };
};

template <typename T>
concept readable_method = requires(Reader& reader, T& value) {
  { value.read(reader) };
};
template <readable_method T>
inline void read(Reader& reader, T& value) {
  value.read(reader);
}

template <typename T>
concept serializable = readable<T> && writeable<T>;

// Writer
class Writer {
public:
  Writer() {}

  // Write values

  template <writeable T>
  void value(const T& value) {
    write(*this, value);
  }

  template <writeable T>
  void value(const char* key, const T& value) {
    object_next(key);
    write(*this, value);
  }

  // Primitives

  virtual void number(NumberType type, const void* value) = 0;
  virtual void boolean(bool value) = 0;
  virtual void string(const char* string) = 0;
  virtual void enumerate(int value, const std::span<const char*>& labels) = 0;
  virtual void binary(const std::span<const std::uint8_t>& data) = 0;

  // Single-element containers

  virtual void optional_begin(bool has_value) = 0;
  virtual void optional_end() = 0;

  virtual void variant_begin(int value, const std::span<const char*>& labels) = 0;
  virtual void variant_end() = 0;

  // Fixed-size containers

  virtual void object_begin() = 0;
  virtual void object_next(const char* key) = 0;
  virtual void object_end() = 0;

  virtual void tuple_begin() = 0;
  virtual void tuple_next() = 0;
  virtual void tuple_end() = 0;

  // Variable-size containers

  virtual void list_begin(size_t size) = 0;
  virtual void list_next() = 0;
  virtual void list_end() = 0;

  // Dummy methods to support the same method calls as Reader()
  // in macro generated read/write functions

  void hint(const Hint&) {
    // Do nothing
  }

  void description(const std::string&) {
    // Do nothing
  }
};

// Reader
class Reader {
public:
  Reader(bool is_tokenizer = false) : valid_(true), is_tokenizer_(is_tokenizer) {}

  template <readable T>
  void value(T& value) {
    read(*this, value);
  }

  template <readable T>
  void value(const char* key, T& value) {
    object_next(key);
    read(*this, value);
  }

  // Primitives

  virtual void number(NumberType type, void* value) = 0;
  virtual bool boolean() = 0;
  virtual const char* string() = 0;
  virtual int enumerate(const std::span<const char*>& labels) = 0;
  virtual std::span<const std::uint8_t> binary() = 0;

  // Single-element containers

  virtual bool optional_begin() = 0;
  virtual void optional_end() = 0;

  virtual int variant_begin(const std::span<const char*>& labels) = 0;
  virtual void variant_tokenize(int index) {}; // Only used by tokenizer
  virtual void variant_end() = 0;

  // Fixed-size containers

  virtual void object_begin() = 0;
  virtual void object_next(const char* key) = 0;
  virtual void object_end() = 0;

  virtual void tuple_begin() = 0;
  virtual void tuple_next() = 0;
  virtual void tuple_end() = 0;

  // Variable-size containers

  virtual size_t list_begin() = 0;
  virtual void list_next() = 0;
  virtual void list_end() = 0;

  // Other

  void invalidate() {
    valid_ = false;
  }
  bool valid() const {
    return valid_;
  }
  bool is_tokenizer() const {
    return is_tokenizer_;
  }

  virtual void hint(const Hint& hint) {}
  virtual void description(const std::string& description) {}

private:
  bool valid_;
  const bool is_tokenizer_;
};

#define DPACK_NUMBER(Type, Enum)                                                                   \
  inline void write(Writer& writer, const Type& value) {                                           \
    writer.number(NumberType::Enum, &value);                                                       \
  }                                                                                                \
  inline void read(Reader& reader, Type& value) {                                                  \
    reader.number(NumberType::Enum, &value);                                                       \
  }

DPACK_NUMBER(std::int32_t, I32);
DPACK_NUMBER(std::int64_t, I64);
DPACK_NUMBER(std::uint32_t, U32);
DPACK_NUMBER(std::uint64_t, U64);
DPACK_NUMBER(std::uint8_t, U8);
DPACK_NUMBER(float, F32);
DPACK_NUMBER(double, F64);

#undef DPACK_NUMBER

inline void write(Writer& writer, const bool& value) {
  writer.boolean(value);
}
inline void read(Reader& reader, bool& value) {
  value = reader.boolean();
}

template <labelled_enum T>
void write(Writer& writer, const T& value) {
  writer.enumerate((int)value, enum_labels<T>);
}

template <labelled_enum T>
void read(Reader& reader, T& value) {
  value = (T)reader.enumerate(enum_labels<T>);
}

// Nasty macro magic!
// Hopefully this can be replaced by C++ reflection at some point
// https://stackoverflow.com/a/11994395

#define _DPACK_FE_0(WHAT)
#define _DPACK_FE_1(WHAT, X) WHAT(X)
#define _DPACK_FE_2(WHAT, X, ...) WHAT(X) _DPACK_FE_1(WHAT, __VA_ARGS__)
#define _DPACK_FE_3(WHAT, X, ...) WHAT(X) _DPACK_FE_2(WHAT, __VA_ARGS__)
#define _DPACK_FE_4(WHAT, X, ...) WHAT(X) _DPACK_FE_3(WHAT, __VA_ARGS__)
#define _DPACK_FE_5(WHAT, X, ...) WHAT(X) _DPACK_FE_4(WHAT, __VA_ARGS__)
#define _DPACK_FE_6(WHAT, X, ...) WHAT(X) _DPACK_FE_5(WHAT, __VA_ARGS__)
#define _DPACK_FE_7(WHAT, X, ...) WHAT(X) _DPACK_FE_6(WHAT, __VA_ARGS__)
#define _DPACK_FE_8(WHAT, X, ...) WHAT(X) _DPACK_FE_7(WHAT, __VA_ARGS__)
#define _DPACK_FE_9(WHAT, X, ...) WHAT(X) _DPACK_FE_8(WHAT, __VA_ARGS__)
#define _DPACK_FE_10(WHAT, X, ...) WHAT(X) _DPACK_FE_9(WHAT, __VA_ARGS__)
#define _DPACK_FE_11(WHAT, X, ...) WHAT(X) _DPACK_FE_10(WHAT, __VA_ARGS__)
#define _DPACK_FE_12(WHAT, X, ...) WHAT(X) _DPACK_FE_11(WHAT, __VA_ARGS__)

#define _DPACK_GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, NAME, ...) NAME
#define _DPACK_FOR_EACH(action, ...)                                                               \
  _DPACK_GET_MACRO(                                                                                \
      _0 __VA_OPT__(, ) __VA_ARGS__,                                                               \
      _DPACK_FE_12,                                                                                \
      _DPACK_FE_11,                                                                                \
      _DPACK_FE_10,                                                                                \
      _DPACK_FE_9,                                                                                 \
      _DPACK_FE_8,                                                                                 \
      _DPACK_FE_7,                                                                                 \
      _DPACK_FE_6,                                                                                 \
      _DPACK_FE_5,                                                                                 \
      _DPACK_FE_4,                                                                                 \
      _DPACK_FE_3,                                                                                 \
      _DPACK_FE_2,                                                                                 \
      _DPACK_FE_1,                                                                                 \
      _DPACK_FE_0)                                                                                 \
  (action __VA_OPT__(, ) __VA_ARGS__)

#define _DPACK_VALUE(X) packer.value(#X, value.X);
#define _DPACK_CLASS_VALUE(X) packer.value(#X, X);

// https://stackoverflow.com/a/62984543
// Needs to support both:
//   _DPACK_DEPAREN((x, y)) -> x, y   Does have parentheses
//   _DPACK_DEPAREN(x)      -> x      Doesn't have parentheses
#define _DPACK_DEPAREN(X) _DPACK_REMOVE_DP_PAREN(_DP_PAREN X)
#define _DP_PAREN(...) _DP_PAREN __VA_ARGS__
#define _DPACK_REMOVE_DP_PAREN(...) _DPACK_REMOVE_DP_PAREN2(__VA_ARGS__)
#define _DPACK_REMOVE_DP_PAREN2(...) _IGNORE##__VA_ARGS__
#define _IGNORE_DP_PAREN

// ===================================================================================
// Free function macros

#define DPACK_DECL(Type)                                                                           \
  void read(Reader& packer, Type& value);                                                          \
  void write(Writer& packer, const Type& value);

#define DPACK_DEF(Type, ...)                                                                       \
  void read(Reader& packer, Type& value) {                                                         \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_VALUE, __VA_ARGS__)                                                     \
    packer.object_end();                                                                           \
  }                                                                                                \
  void write(Writer& packer, const Type& value) {                                                  \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_VALUE, __VA_ARGS__)                                                     \
    packer.object_end();                                                                           \
  }

#define DPACK_DEF_CUSTOM(Type, ...)                                                                \
  void read(Reader& packer, Type& value) {                                                         \
    __VA_ARGS__;                                                                                   \
  }                                                                                                \
  void write(Writer& packer, const Type& value) {                                                  \
    __VA_ARGS__;                                                                                   \
  }

#define DPACK_INLINE(Type, ...)                                                                    \
  inline void read(Reader& packer, Type& value) {                                                  \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_VALUE, __VA_ARGS__)                                                     \
    packer.object_end();                                                                           \
  }                                                                                                \
  inline void write(Writer& packer, const Type& value) {                                           \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_VALUE, __VA_ARGS__)                                                     \
    packer.object_end();                                                                           \
  }

#define DPACK_INLINE_CUSTOM(Type, ...)                                                             \
  inline void read(Reader& packer, Type& value) {                                                  \
    __VA_ARGS__;                                                                                   \
  }                                                                                                \
  inline void write(Writer& packer, const Type& value) {                                           \
    __VA_ARGS__;                                                                                   \
  }

// ===================================================================================
// Class macros

#define DPACK_CLASS_DECL()                                                                         \
  void read(::dpack::Reader& packer);                                                              \
  void write(::dpack::Writer& packer) const;

#define DPACK_CLASS_DEF(Class, ...)                                                                \
  void Class::read(::dpack::Reader& packer) {                                                      \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_CLASS_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }                                                                                                \
  void Class::write(::dpack::Writer& packer) const {                                               \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_CLASS_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }

#define DPACK_CLASS_DEF_CUSTOM(Class, ...)                                                         \
  void Class::read(::dpack::Reader& packer) {                                                      \
    __VA_ARGS__;                                                                                   \
  }                                                                                                \
  void Class::write(::dpack::Writer& packer) const {                                               \
    __VA_ARGS__;                                                                                   \
  }

#define DPACK_CLASS_INLINE(...)                                                                    \
  void read(::dpack::Reader& packer) {                                                             \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_CLASS_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }                                                                                                \
  void write(::dpack::Writer& packer) const {                                                      \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_CLASS_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }

#define DPACK_CLASS_INLINE_CUSTOM(...)                                                             \
  void read(::dpack::Reader& packer) {                                                             \
    __VA_ARGS__;                                                                                   \
  }                                                                                                \
  void write(::dpack::Writer& packer) const {                                                      \
    __VA_ARGS__;                                                                                   \
  }

// ===================================================================================
// Templated macros

#define DPACK_TEMPLATED_INLINE(Type, Typenames, ...)                                               \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void read(Reader& packer, _DPACK_DEPAREN(Type) & value) {                                        \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_VALUE, __VA_ARGS__)                                                     \
    packer.object_end();                                                                           \
  }                                                                                                \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void write(Writer& packer, const _DPACK_DEPAREN(Type) & value) {                                 \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_VALUE, __VA_ARGS__)                                                     \
    packer.object_end();                                                                           \
  }

#define DPACK_TEMPLATED_INLINE_CUSTOM(Type, Typenames, ...)                                        \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void read(Reader& packer, _DPACK_DEPAREN(Type) & value) {                                        \
    __VA_ARGS__;                                                                                   \
  }                                                                                                \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void write(Writer& packer, const _DPACK_DEPAREN(Type) & value) {                                 \
    __VA_ARGS__;                                                                                   \
  }

#define DPACK_TEMPLATED_DECL(Type, Typenames)                                                      \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void read(Reader& packer, _DPACK_DEPAREN(Type) & value);                                         \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void write(Writer& packer, const _DPACK_DEPAREN(Type) & value);

#define DPACK_TEMPLATED_DEF(Type, Typenames, ...)                                                  \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void read(Reader& packer, _DPACK_DEPAREN(Type) & value) {                                        \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_VALUE, __VA_ARGS__)                                                     \
    packer.object_end();                                                                           \
  }                                                                                                \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void write(Writer& packer, const _DPACK_DEPAREN(Type) & value) {                                 \
    packer.object_begin();                                                                         \
    _DPACK_FOR_EACH(_DPACK_VALUE, __VA_ARGS__)                                                     \
    packer.object_end();                                                                           \
  }

#define DPACK_TEMPLATED_DEF_CUSTOM(Type, Typenames, ...)                                           \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void read(Reader& packer, _DPACK_DEPAREN(Type) & value) {                                        \
    __VA_ARGS__;                                                                                   \
  }                                                                                                \
  template <_DPACK_DEPAREN(Typenames)>                                                             \
  void write(Writer& packer, const _DPACK_DEPAREN(Type) & value) {                                 \
    __VA_ARGS__;                                                                                   \
  }

#define DPACK_TEMPLATED_INSTANTIATE(Type, ...)                                                     \
  template void read<__VA_ARGS__>(Reader & packer, Type<__VA_ARGS__> & value);                     \
  template void write<__VA_ARGS__>(Writer & packer, const Type<__VA_ARGS__>& value);

} // namespace dpack
