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
concept writeable = requires(const T& value, Writer& writer) {
  { write(value, writer) };
};

class Writeable {
public:
  virtual void write(Writer& writer) const = 0;
};

// Writer
class Writer {
public:
  Writer() {}

  // Write values

  template <writeable T>
  void value(const T& value) {
    pack(value, *this);
  }

  template <writeable T>
  void value(const char* key, const T& value) {
    object_next(key);
    pack(value, *this);
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

class Reader;

template <typename T>
concept readable = requires(T& value, Reader& reader) {
  { read(value, reader) };
};

class Readable {
public:
  virtual void read(Reader& reader) = 0;
};

// Reader
class Reader {
public:
  Reader(bool is_tokenizer = false) : valid_(true), is_tokenizer_(is_tokenizer) {}

  template <readable T>
  void value(T& value) {
    pack(value, *this);
  }

  template <readable T>
  void value(const char* key, T& value) {
    object_next(key);
    pack(value, *this);
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
  inline void write(const Type& value, Writer& writer) {                                           \
    writer.number(NumberType::Enum, &value);                                                       \
  }                                                                                                \
  inline void read(Type& value, Reader& reader) {                                                  \
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

inline void write(const bool& value, Writer& writer) {
  writer.boolean(value);
}
inline void read(bool& value, Reader& reader) {
  value = reader.boolean();
}

template <labelled_enum T>
void write(const T& value, Writer& writer) {
  writer.enumerate((int)value, enum_labels<T>);
}

template <labelled_enum T>
void read(T& value, Reader& reader) {
  value = (T)reader.enumerate(enum_labels<T>);
}

inline void write(const Writeable& value, Writer& writer) {
  value.write(writer);
}

inline void read(Readable& value, Reader& reader) {
  value.read(reader);
}

} // namespace datapack
