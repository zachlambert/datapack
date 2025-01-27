#pragma once

#include "datapack/constraint.hpp"
#include "datapack/number.hpp"
#include "datapack/packer.hpp"
#include <cstdint>
#include <span>
#include <string>
#include <span>
#include <vector>
#include <optional>
#include <tuple>


namespace datapack {

template <> class Packer<MODE_READ> {
public:
  Packer(bool trivial_as_binary = false, bool is_tokenizer = false,
         bool check_constraints = false)
      : valid_(true), trivial_as_binary_(trivial_as_binary),
        is_tokenizer_(is_tokenizer), check_constraints_(check_constraints),
        constraint_(nullptr) {}

  template <readable T> void value(T &value) { pack(value, *this); }

  template <readable T> void value(const char *key, T &value) {
    object_next(key);
    this->value(value);
  }

  template <readable T, typename Constraint>
    requires is_constrained<T, Constraint>
  void value(T &value, const Constraint &constraint) {
    constraint_ = &constraint;
    pack(value, *this);
    if (!is_tokenizer_ && !constraint.validate(value)) {
      valid_ = false;
    }
    constraint_ = nullptr;
  }

  template <readable T, typename Constraint>
    requires is_constrained<T, Constraint>
  void value(const char *key, T &value, const Constraint &constraint) {
    object_next(key);
    this->value(value, constraint);
  }

  // Primitives

  virtual void integer(IntType type, void *value) = 0;
  virtual void floating(FloatType type, void *value) = 0;
  virtual bool boolean() = 0;
  virtual const char *string() = 0;
  virtual int enumerate(const std::span<const char *> &labels) = 0;
  virtual std::tuple<const std::uint8_t *, std::size_t>
  binary(std::size_t length, std::size_t stride) = 0;

  // Single-element containers

  virtual bool optional_begin() = 0;
  virtual void optional_end() = 0;

  virtual int variant_begin(const std::span<const char *> &labels) = 0;
  virtual void variant_tokenize(int index) {}; // Only used by tokenizer
  virtual void variant_end() = 0;

  // Fixed-size containers

  virtual void object_begin(std::size_t size = 0) = 0;
  virtual void object_next(const char *key) = 0;
  virtual void object_end(std::size_t size = 0) = 0;

  virtual void tuple_begin(std::size_t size = 0) = 0;
  virtual void tuple_next() = 0;
  virtual void tuple_end(std::size_t size = 0) = 0;

  // Variable-size containers

  virtual void list_begin(bool is_trivial = false) = 0;
  virtual bool list_next() = 0;
  virtual void list_end() = 0;

  // Other

  void invalidate() { valid_ = false; }
  bool valid() const { return valid_; }
  bool trivial_as_binary() const { return trivial_as_binary_; }
  bool is_tokenizer() const { return is_tokenizer_; }
  bool check_constraints() const { return check_constraints_; }

  template <typename Constraint>
    requires std::is_base_of_v<ConstraintBase, Constraint>
  const Constraint *constraint() const {
    return dynamic_cast<const Constraint *>(constraint_);
  }

private:
  bool valid_;
  const bool trivial_as_binary_;
  const bool is_tokenizer_;
  const bool check_constraints_;
  const ConstraintBase *constraint_;
};

} // namespace datapack
