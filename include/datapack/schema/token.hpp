#pragma once

#include "datapack/datapack.hpp"
#include "datapack/hint.hpp"
#include "datapack/labelled_enum.hpp"
#include "datapack/labelled_variant.hpp"
#include "datapack/std/optional.hpp"
#include "datapack/std/string.hpp"
#include "datapack/std/variant.hpp"
#include "datapack/std/vector.hpp"
#include <string>
#include <vector>

namespace dpack {

DPACK_LABELLED_ENUM(NumberType, 7);

// Cannot put these in the hint header, since this is included by datapack.hpp
DPACK_INLINE(HintChoices, choices);
DPACK_INLINE(HintRange, lower, upper);
DPACK_INLINE(HintPositive, allow_zero);
DPACK_INLINE(HintColor);
DPACK_LABELLED_VARIANT(Hint, 4);

namespace token {

struct Number {
  NumberType type;

  explicit Number() {}
  explicit Number(NumberType type) : type(type) {}

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

struct Binary {};
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

struct ObjectBegin {};
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

struct Hint {
  ::dpack::Hint hint;
};

struct Description {
  std::string description;
};

} // namespace token

DPACK_INLINE(token::Number, type)
DPACK_INLINE(token::Boolean)
DPACK_INLINE(token::String)
DPACK_INLINE(token::Enumerate, labels)
DPACK_INLINE(token::Binary)
DPACK_INLINE(token::Optional)
DPACK_INLINE(token::VariantBegin, labels)
DPACK_INLINE(token::VariantNext, index)
DPACK_INLINE(token::VariantEnd)
DPACK_INLINE(token::ObjectBegin)
DPACK_INLINE(token::ObjectNext, key)
DPACK_INLINE(token::ObjectEnd)
DPACK_INLINE(token::TupleBegin)
DPACK_INLINE(token::TupleNext)
DPACK_INLINE(token::TupleEnd)
DPACK_INLINE(token::List)
DPACK_INLINE(token::Hint, hint)
DPACK_INLINE(token::Description, description)

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
    token::List,
    token::Hint,
    token::Description>;
DPACK_LABELLED_VARIANT(Token, 18);

bool operator==(const Token& lhs, const Token& rhs);

} // namespace dpack
