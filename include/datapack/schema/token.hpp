#pragma once

#include "datapack/datapack.hpp"
#include "datapack/labelled_enum.hpp"
#include "datapack/labelled_variant.hpp"
#include <string>
#include <vector>

namespace datapack {

namespace token {

struct Enumerate {
  std::vector<std::string> labels;
  Enumerate() {}
  Enumerate(const std::vector<std::string>& labels) : labels(labels) {}
  Enumerate(const std::span<const char*>& labels) {
    for (const char* str : labels) {
      this->labels.push_back(std::string(str));
    }
  }
};

struct Binary {
  std::size_t length;
  std::size_t stride;
  Binary() : length(0), stride(0) {}
  Binary(std::size_t length, std::size_t stride) : length(length), stride(stride) {}
};

struct Optional {};

struct VariantBegin {
  std::vector<std::string> labels;
  VariantBegin() {}
  VariantBegin(const std::vector<std::string>& labels) : labels(labels) {}
  VariantBegin(const std::span<const char*>& labels) {
    for (const char* str : labels) {
      this->labels.push_back(std::string(str));
    }
  }
};
struct VariantEnd {};
struct VariantNext {
  int index;
  VariantNext() {}
  VariantNext(int index) : index(index) {}
};

struct ObjectBegin {};
struct ObjectEnd {};
struct ObjectNext {
  std::string key;
  ObjectNext() {}
  ObjectNext(const std::string& key) : key(key) {}
};

struct TupleBegin {};
struct TupleEnd {};
struct TupleNext {};

struct List {};

} // namespace token

using Token = std::variant<
    NumberType,
    bool,
    std::string,
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
DATAPACK(token::Enumerate);
DATAPACK(token::Binary);

DATAPACK_EMPTY(token::Optional);
DATAPACK(token::VariantBegin);
DATAPACK_EMPTY(token::VariantEnd);
DATAPACK(token::VariantNext);

DATAPACK_EMPTY(token::ObjectBegin);
DATAPACK_EMPTY(token::ObjectEnd);
DATAPACK(token::ObjectNext);

DATAPACK_EMPTY(token::TupleNext);
DATAPACK_EMPTY(token::TupleBegin);
DATAPACK_EMPTY(token::TupleEnd);

DATAPACK_EMPTY(token::List);

DATAPACK_LABELLED_VARIANT(Token, 16);

bool operator==(const Token& lhs, const Token& rhs);

} // namespace datapack
