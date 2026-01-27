#pragma once

#include "datapack/constraint.hpp"
#include "datapack/labelled_enum.hpp"
#include <concepts>
#include <cstdint>
#include <optional>
#include <span>

namespace datapack {

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
concept supported = readable<T> && writeable<T>;

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

  virtual void list_begin() = 0;
  virtual void list_next() = 0;
  virtual void list_end() = 0;

  // Other

  void constraint(const Constraint& constraint) {
    constraint_ = constraint;
  }

protected:
  template <typename T>
  std::optional<T> get_constraint() {
    if (!constraint_.has_value()) {
      return std::nullopt;
    }
    if (std::get_if<T>(&(*constraint_))) {
      return std::move(std::get<T>(*constraint_));
    }
    return std::nullopt;
  }

private:
  std::optional<Constraint> constraint_;
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

  virtual void list_begin() = 0;
  virtual bool list_next() = 0;
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

  void constraint(const Constraint& constraint) {
    constraint_ = constraint;
  }

protected:
  template <typename T>
  std::optional<T> get_constraint() {
    if (!constraint_.has_value()) {
      return std::nullopt;
    }
    if (std::get_if<T>(&(*constraint_))) {
      return std::move(std::get<T>(*constraint_));
    }
    return std::nullopt;
  }

private:
  bool valid_;
  const bool is_tokenizer_;
  std::optional<Constraint> constraint_;
};

#define DATAPACK_NUMBER(Type, Enum)                                                                \
  inline void write(Writer& writer, const Type& value) {                                           \
    writer.number(NumberType::Enum, &value);                                                       \
  }                                                                                                \
  inline void read(Reader& reader, Type& value) {                                                  \
    reader.number(NumberType::Enum, &value);                                                       \
  }

DATAPACK_NUMBER(std::int32_t, I32);
DATAPACK_NUMBER(std::int64_t, I64);
DATAPACK_NUMBER(std::uint32_t, U32);
DATAPACK_NUMBER(std::uint64_t, U64);
DATAPACK_NUMBER(std::uint8_t, U8);
DATAPACK_NUMBER(float, F32);
DATAPACK_NUMBER(double, F64);

#undef DATAPACK_NUMBER

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

#define _DATAPACK_FE_0(WHAT)
#define _DATAPACK_FE_1(WHAT, X) WHAT(X)
#define _DATAPACK_FE_2(WHAT, X, ...) WHAT(X) _DATAPACK_FE_1(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_3(WHAT, X, ...) WHAT(X) _DATAPACK_FE_2(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_4(WHAT, X, ...) WHAT(X) _DATAPACK_FE_3(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_5(WHAT, X, ...) WHAT(X) _DATAPACK_FE_4(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_6(WHAT, X, ...) WHAT(X) _DATAPACK_FE_5(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_7(WHAT, X, ...) WHAT(X) _DATAPACK_FE_6(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_8(WHAT, X, ...) WHAT(X) _DATAPACK_FE_7(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_9(WHAT, X, ...) WHAT(X) _DATAPACK_FE_8(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_10(WHAT, X, ...) WHAT(X) _DATAPACK_FE_9(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_11(WHAT, X, ...) WHAT(X) _DATAPACK_FE_10(WHAT, __VA_ARGS__)
#define _DATAPACK_FE_12(WHAT, X, ...) WHAT(X) _DATAPACK_FE_11(WHAT, __VA_ARGS__)

#define _DATAPACK_GET_MACRO(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, NAME, ...) NAME
#define _DATAPACK_FOR_EACH(action, ...)                                                            \
  _DATAPACK_GET_MACRO(                                                                             \
      _0 __VA_OPT__(, ) __VA_ARGS__,                                                               \
      _DATAPACK_FE_12,                                                                             \
      _DATAPACK_FE_11,                                                                             \
      _DATAPACK_FE_10,                                                                             \
      _DATAPACK_FE_9,                                                                              \
      _DATAPACK_FE_8,                                                                              \
      _DATAPACK_FE_7,                                                                              \
      _DATAPACK_FE_6,                                                                              \
      _DATAPACK_FE_5,                                                                              \
      _DATAPACK_FE_4,                                                                              \
      _DATAPACK_FE_3,                                                                              \
      _DATAPACK_FE_2,                                                                              \
      _DATAPACK_FE_1,                                                                              \
      _DATAPACK_FE_0)                                                                              \
  (action __VA_OPT__(, ) __VA_ARGS__)

#define _DATAPACK_VALUE(X) packer.value(#X, value.X);
#define _DATAPACK_CLASS_VALUE(X) packer.value(#X, X);

// https://stackoverflow.com/a/62984543
// Needs to support both:
//   _DATAPACK_DEPAREN((x, y)) -> x, y   Does have parentheses
//   _DATAPACK_DEPAREN(x)      -> x      Doesn't have parentheses
#define _DATAPACK_DEPAREN(X) _DATAPACK_REMOVE_DP_PAREN(_DP_PAREN X)
#define _DP_PAREN(...) _DP_PAREN __VA_ARGS__
#define _DATAPACK_REMOVE_DP_PAREN(...) _DATAPACK_REMOVE_DP_PAREN2(__VA_ARGS__)
#define _DATAPACK_REMOVE_DP_PAREN2(...) _IGNORE##__VA_ARGS__
#define _IGNORE_DP_PAREN

// ===================================================================================
// Free function macros

#define DATAPACK_DECL(Type)                                                                        \
  void read(Reader& packer, Type& value);                                                          \
  void write(Writer& packer, const Type& value);

#define DATAPACK_DEF(Type, ...)                                                                    \
  inline void read(Reader& packer, Type& value) {                                                  \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }                                                                                                \
  inline void write(Writer& packer, const Type& value) {                                           \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }

#define DATAPACK_INLINE(Type, ...)                                                                 \
  inline void read(Reader& packer, Type& value) {                                                  \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }                                                                                                \
  inline void write(Writer& packer, const Type& value) {                                           \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }

// ===================================================================================
// Class macros

#define DATAPACK_CLASS_DECL()                                                                      \
  void read(::datapack::Reader& packer);                                                           \
  void write(::datapack::Writer& packer) const;

#define DATAPACK_CLASS_INLINE(...)                                                                 \
  inline void read(::datapack::Reader& packer) {                                                   \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_CLASS_VALUE, __VA_ARGS__)                                         \
    packer.object_end();                                                                           \
  }                                                                                                \
  inline void write(::datapack::Writer& packer) const {                                            \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_CLASS_VALUE, __VA_ARGS__)                                         \
    packer.object_end();                                                                           \
  }

#define DATAPACK_CLASS_DEF(Class, ...)                                                             \
  void Class::read(::datapack::Reader& packer) {                                                   \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_CLASS_VALUE, __VA_ARGS__)                                         \
    packer.object_end();                                                                           \
  }                                                                                                \
  void Class::write(::datapack::Writer& packer) const {                                            \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_CLASS_VALUE, __VA_ARGS__)                                         \
    packer.object_end();                                                                           \
  }
//
// ===================================================================================
// Templated macros

#define DATAPACK_TEMPLATED_INLINE(Typenames, Type, ...)                                            \
  template <_DATAPACK_DEPAREN(Typenames)>                                                          \
  void read(Reader& packer, _DATAPACK_DEPAREN(Type) & value) {                                     \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }                                                                                                \
  template <_DATAPACK_DEPAREN(Typenames)>                                                          \
  void write(Writer& packer, const _DATAPACK_DEPAREN(Type) & value) {                              \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }

#define DATAPACK_TEMPLATED_DECL(Typenames, Type)                                                   \
  template <_DATAPACK_DEPAREN(Typenames)>                                                          \
  void read(Reader& packer, _DATAPACK_DEPAREN(Type) & value);                                      \
  template <_DATAPACK_DEPAREN(Typenames)>                                                          \
  void write(Writer& packer, const _DATAPACK_DEPAREN(Type) & value);

#define DATAPACK_TEMPLATED_DEF(Typenames, Type, ...)                                               \
  template <_DATAPACK_DEPAREN(Typenames)>                                                          \
  void read(Reader& packer, _DATAPACK_DEPAREN(Type) & value) {                                     \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }                                                                                                \
  template <_DATAPACK_DEPAREN(Typenames)>                                                          \
  void write(Writer& packer, const _DATAPACK_DEPAREN(Type) & value) {                              \
    packer.object_begin();                                                                         \
    _DATAPACK_FOR_EACH(_DATAPACK_VALUE, __VA_ARGS__)                                               \
    packer.object_end();                                                                           \
  }

#define DATAPACK_TEMPLATED_INSTANTIATE(Type, Templates)                                            \
  template void read<_DATAPACK_DEPAREN(Templates)>(                                                \
      Reader & packer,                                                                             \
      Type<_DATAPACK_DEPAREN(Templates)> & value);                                                 \
  template void write<_DATAPACK_DEPAREN(Templates)>(                                               \
      Writer & packer,                                                                             \
      const Type<_DATAPACK_DEPAREN(Templates)>& value);

} // namespace datapack
