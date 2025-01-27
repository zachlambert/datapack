#pragma once

#include "datapack/constraint.hpp"
#include "datapack/number.hpp"
#include "datapack/packer.hpp"
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <vector>
#include <tuple>

namespace datapack {

template <>
class Packer<MODE_WRITE> {
public:
  Packer(bool trivial_as_binary = false) : trivial_as_binary_(trivial_as_binary) {}

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

  template <writeable T, typename Constraint>
  void value(const T& value, const Constraint& constraint) {
    // constraint unused by Writer
    pack(value, *this);
  }

  template <writeable T, typename Constraint>
  void value(const char* key, const T& value, const Constraint& constraint) {
    // constraint unused by Writer
    object_next(key);
    this->value(value);
  }

  // Primitives

  virtual void integer(IntType type, const void* value) = 0;
  virtual void floating(FloatType type, const void* value) = 0;
  virtual void boolean(bool value) = 0;
  virtual void string(const char* string) = 0;
  virtual void enumerate(int value, const char* label) = 0;
  virtual void binary(
      const std::uint8_t* data,
      std::size_t length,
      std::size_t stride,
      bool fixed_length) = 0;

  // Single-element containers

  virtual void optional_begin(bool has_value) = 0;
  virtual void optional_end() = 0;

  virtual void variant_begin(int value, const char* label) = 0;
  virtual void variant_end() = 0;

  // Fixed-size containers

  virtual void object_begin(std::size_t size = 0) = 0;
  virtual void object_next(const char* key) = 0;
  virtual void object_end(std::size_t size = 0) = 0;

  virtual void tuple_begin(std::size_t size = 0) = 0;
  virtual void tuple_next() = 0;
  virtual void tuple_end(std::size_t size = 0) = 0;

  // Variable-size containers

  virtual void list_begin(bool is_trivial = false) = 0;
  virtual void list_next() = 0;
  virtual void list_end() = 0;

  // Other

  bool trivial_as_binary() const { return trivial_as_binary_; }

private:
  const bool trivial_as_binary_;
};

} // namespace datapack
