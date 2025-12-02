#pragma once

#include "datapack/constraint.hpp"
#include "datapack/datapack.hpp"
#include "datapack/labelled_enum.hpp"
#include "datapack/labelled_variant.hpp"
#include <string>
#include <vector>

namespace datapack {

namespace token {

struct Number {
  NumberType type;
  std::optional<ConstraintNumber> constraint;
  explicit Number() {}
  explicit Number(NumberType type) : type(type) {}
  explicit Number(NumberType type, ConstraintNumber&& constraint) :
      type(type), constraint(std::move(constraint)) {}

  static Number I32() {
    return Number(NumberType::I32);
  }
  static Number I64() {
    return Number(NumberType::I64);
  }
  static Number U32() {
    return Number(NumberType::U32);
  }
  static Number U64() {
    return Number(NumberType::U64);
  }
  static Number F32() {
    return Number(NumberType::F32);
  }
  static Number F64() {
    return Number(NumberType::F64);
  }
  static Number U8() {
    return Number(NumberType::U8);
  }
};

struct Boolean {};

struct String {};

struct Enumerate {
  std::vector<std::string> labels;
  explicit Enumerate() {}
  explicit Enumerate(const std::vector<std::string>& labels) : labels(labels) {}
  explicit Enumerate(const std::span<const char*>& labels) {
    for (const char* str : labels) {
      this->labels.push_back(std::string(str));
    }
  }
};

struct Binary {
  std::size_t length;
  std::size_t stride;
  explicit Binary() : length(0), stride(0) {}
  explicit Binary(std::size_t length, std::size_t stride) : length(length), stride(stride) {}
};

struct Optional {};

struct VariantBegin {
  std::vector<std::string> labels;
  explicit VariantBegin() {}
  explicit VariantBegin(const std::vector<std::string>& labels) : labels(labels) {}
  explicit VariantBegin(const std::span<const char*>& labels) {
    for (const char* str : labels) {
      this->labels.push_back(std::string(str));
    }
  }
};
struct VariantEnd {};
struct VariantNext {
  int index;
  explicit VariantNext() {}
  explicit VariantNext(int index) : index(index) {}
};

struct ObjectBegin {
  std::optional<ConstraintObject> constraint;
  ObjectBegin() {}
  ObjectBegin(ConstraintObject&& constraint) : constraint(std::move(constraint)) {}
};
struct ObjectEnd {};
struct ObjectNext {
  std::string key;
  explicit ObjectNext() {}
  explicit ObjectNext(const std::string& key) : key(key) {}
};

struct TupleBegin {};
struct TupleEnd {};
struct TupleNext {};

struct List {};

} // namespace token

using Token = std::variant<
    token::Number,
    token::Boolean,
    token::String,
    token::Enumerate,
    token::Binary,
    token::Optional,
    token::VariantBegin,
    token::VariantNext,
    token::VariantEnd,
    token::ObjectBegin,
    token::ObjectNext,
    token::ObjectEnd,
    token::TupleBegin,
    token::TupleNext,
    token::TupleEnd,
    token::List>;

DATAPACK_LABELLED_ENUM(NumberType, 7);
DATAPACK(token::Number);
DATAPACK_EMPTY(token::Boolean);
DATAPACK_EMPTY(token::String);
DATAPACK(token::Enumerate);
DATAPACK(token::Binary);

DATAPACK_EMPTY(token::Optional);
DATAPACK(token::VariantBegin);
DATAPACK_EMPTY(token::VariantEnd);
DATAPACK(token::VariantNext);

DATAPACK(token::ObjectBegin);
DATAPACK_EMPTY(token::ObjectEnd);
DATAPACK(token::ObjectNext);

DATAPACK_EMPTY(token::TupleNext);
DATAPACK_EMPTY(token::TupleBegin);
DATAPACK_EMPTY(token::TupleEnd);

DATAPACK_EMPTY(token::List);

DATAPACK_LABELLED_VARIANT(Token, 16);

bool operator==(const Token& lhs, const Token& rhs);

// Cannot put these in the constraint header, since this is included by datapack.hpp

DATAPACK_LABELLED_VARIANT(Constraint, 2);
DATAPACK_LABELLED_VARIANT(ConstraintObject, 1);
DATAPACK_LABELLED_VARIANT(ConstraintNumber, 1);

DATAPACK_EMPTY(ConstraintObjectColor);
DATAPACK(ConstraintNumberRange);

} // namespace datapack
