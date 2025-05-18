#pragma once

#include <concepts>
#include <cstdint>
#include <span>

namespace datapack {

enum class NumberType { I32, I64, U32, U64, U8, F32, F64 };

template <int Mode>
class Packer;

static constexpr int MODE_WRITE = 0;
static constexpr int MODE_READ = 1;

template <typename T, int Mode>
using packref = std::conditional_t<Mode == 0, const T&, T&>;

template <typename T, int Mode>
concept packable = requires(packref<T, Mode> value, Packer<Mode>& packer) {
  { pack(value, packer) };
};

using Writer = Packer<MODE_WRITE>;
template <typename T>
concept writeable = packable<T, MODE_WRITE>;

using Reader = Packer<MODE_READ>;
template <typename T>
concept readable = packable<T, MODE_READ>;

#define DATAPACK(T)                                                                                \
  template <int Mode>                                                                              \
  void pack(packref<T, Mode> value, Packer<Mode>& packer)

#define DATAPACK_IMPL(T, value_name, packer_name)                                                  \
  template void pack(packref<T, MODE_WRITE>, Packer<MODE_WRITE>&);                                 \
  template void pack(packref<T, MODE_READ>, Packer<MODE_READ>&);                                   \
  template <int Mode>                                                                              \
  void pack(packref<T, Mode> value_name, Packer<Mode>& packer_name)

#define DATAPACK_INLINE(T, value_name, packer_name)                                                \
  template <int Mode>                                                                              \
  void pack(packref<T, Mode> value_name, Packer<Mode>& packer_name)

#define DATAPACK_EMPTY(T)                                                                          \
  template <int Mode>                                                                              \
  void pack(packref<T, Mode>, Packer<Mode>& packer) {                                              \
    packer.object_begin();                                                                         \
    packer.object_end();                                                                           \
  }

// Reader
template <>
class Packer<MODE_READ> {
public:
  Packer(bool is_tokenizer = false) : valid_(true), is_tokenizer_(is_tokenizer) {}

  template <readable T>
  void value(T& value) {
    pack(value, *this);
  }

  template <readable T>
  void value(const char* key, T& value) {
    object_next(key);
    this->value(value);
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

  void invalidate() { valid_ = false; }
  bool valid() const { return valid_; }
  bool is_tokenizer() const { return is_tokenizer_; }

private:
  bool valid_;
  const bool is_tokenizer_;
};

// Writer
template <>
class Packer<MODE_WRITE> {
public:
  Packer() {}

  // Write values

  template <writeable T>
  void value(const T& value) {
    pack(value, *this);
  }

  template <writeable T>
  void value(const char* key, const T& value) {
    object_next(key);
    this->value(value);
  }

  // Primitives

  virtual void number(NumberType type, const void* value) = 0;
  virtual void boolean(bool value) = 0;
  virtual void string(const char* string) = 0;
  virtual void enumerate(int value, const char* label) = 0;
  virtual void binary(const std::span<std::uint8_t>& span);

  // Single-element containers

  virtual void optional_begin(bool has_value) = 0;
  virtual void optional_end() = 0;

  virtual void variant_begin(int value, const char* label) = 0;
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
};

DATAPACK_INLINE(std::int32_t, value, packer) { packer.number(NumberType::I32, &value); }
DATAPACK_INLINE(std::int64_t, value, packer) { packer.number(NumberType::I64, &value); }
DATAPACK_INLINE(std::uint32_t, value, packer) { packer.number(NumberType::U32, &value); }
DATAPACK_INLINE(std::uint64_t, value, packer) { packer.number(NumberType::U64, &value); }
DATAPACK_INLINE(std::uint8_t, value, packer) { packer.number(NumberType::U8, &value); }
DATAPACK_INLINE(float, value, packer) { packer.number(NumberType::F32, &value); }
DATAPACK_INLINE(double, value, packer) { packer.number(NumberType::F64, &value); }

inline void pack(const bool& value, Writer& writer) { writer.boolean(value); }
inline void pack(bool& value, Reader& reader) { value = reader.boolean(); }

} // namespace datapack
