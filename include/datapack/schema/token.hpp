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

namespace datapack {

DATAPACK_LABELLED_ENUM(NumberType, 7);

// Cannot put these in the hint header, since this is included by datapack.hpp

DATAPACK_LABELLED_VARIANT(Hint, 3);
DATAPACK_LABELLED_VARIANT(HintObject, 1);
DATAPACK_LABELLED_VARIANT(HintNumber, 1);
DATAPACK_LABELLED_VARIANT(HintString, 1);

DATAPACK_INLINE(HintObjectColor);
DATAPACK_INLINE(HintNumberRange, lower, upper);
DATAPACK_INLINE(HintStringChoices, choices);

namespace token {

struct Number {
  NumberType type;
  std::optional<HintNumber> hint;

  explicit Number() {}
  explicit Number(NumberType type) : type(type) {}
  explicit Number(NumberType type, HintNumber&& hint) : type(type), hint(std::move(hint)) {}

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

struct String {
  std::optional<HintString> hint;

  String() {}
  String(const HintString& hint) : hint(hint) {}
};

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

struct ObjectBegin {
  std::optional<HintObject> hint;
  ObjectBegin() {}
  ObjectBegin(HintObject&& hint) : hint(std::move(hint)) {}
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

DATAPACK_INLINE(token::Number, type, hint)
DATAPACK_INLINE(token::Boolean)
DATAPACK_INLINE(token::String, hint)
DATAPACK_INLINE(token::Enumerate, labels)
DATAPACK_INLINE(token::Binary)
DATAPACK_INLINE(token::Optional)
DATAPACK_INLINE(token::VariantBegin, labels)
DATAPACK_INLINE(token::VariantNext, index)
DATAPACK_INLINE(token::VariantEnd)
DATAPACK_INLINE(token::ObjectBegin, hint)
DATAPACK_INLINE(token::ObjectNext, key)
DATAPACK_INLINE(token::ObjectEnd)
DATAPACK_INLINE(token::TupleBegin)
DATAPACK_INLINE(token::TupleNext)
DATAPACK_INLINE(token::TupleEnd)
DATAPACK_INLINE(token::List)

DATAPACK_LABELLED_VARIANT(Token, 16);

bool operator==(const Token& lhs, const Token& rhs);

} // namespace datapack
