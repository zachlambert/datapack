#pragma once

#include "datapack/packer.hpp"
#include "datapack/primitive.hpp"
#include "datapack/labelled_enum.hpp"
#include "datapack/labelled_variant.hpp"
#include <vector>
#include <string>


namespace datapack {

namespace token {

struct Optional {};

struct Enumerate {
    std::vector<std::string> labels;
    Enumerate() {}
    Enumerate(const std::vector<std::string>& labels): labels(labels) {}
    Enumerate(const std::span<const char*>& labels) {
        for (const char* str: labels) {
            this->labels.push_back(std::string(str));
        }
    }
};
struct VariantBegin {
    std::vector<std::string> labels;
    VariantBegin() {}
    VariantBegin(const std::vector<std::string>& labels): labels(labels) {}
    VariantBegin(const std::span<const char*>& labels) {
        for (const char* str: labels) {
            this->labels.push_back(std::string(str));
        }
    }
};
struct VariantEnd {};
struct VariantNext {
    int index;
    VariantNext() {}
    VariantNext(int index): index(index) {}
};

struct BinaryData {
    std::size_t length;
    std::size_t stride;
    bool fixed_length;
    BinaryData(): length(0), stride(0), fixed_length(false) {}
    BinaryData(std::size_t length, std::size_t stride, bool fixed_length):
        length(length), stride(stride), fixed_length(fixed_length)
    {}
};

struct ObjectBegin {
    std::size_t size;
    ObjectBegin(): size(0) {}
    ObjectBegin(std::size_t size): size(size) {}
};
struct ObjectEnd {
    std::size_t size;
    ObjectEnd(): size(0) {}
    ObjectEnd(std::size_t size): size(size) {}
};
struct ObjectNext {
    std::string key;
    ObjectNext() {}
    ObjectNext(const std::string& key): key(key) {}
};

struct TupleBegin {
    std::size_t size;
    TupleBegin(): size(0) {}
    TupleBegin(std::size_t size): size(size) {}
};
struct TupleEnd {
    std::size_t size;
    TupleEnd(): size(0) {}
    TupleEnd(std::size_t size): size(size) {}
};
struct TupleNext {};

struct List {
    bool is_trivial;
    List(): is_trivial(false) {}
    List(bool is_trivial): is_trivial(is_trivial) {}
};

} // namespace dtoken

using Token = std::variant<
    Primitive,
    std::string,
    token::Enumerate,
    token::Optional,
    token::VariantBegin,
    token::VariantEnd,
    token::VariantNext,
    token::BinaryData,
    token::ObjectBegin,
    token::ObjectEnd,
    token::ObjectNext,
    token::TupleBegin,
    token::TupleEnd,
    token::TupleNext,
    token::List
>;

DATAPACK_LABELLED_ENUM(Primitive, 8);
DATAPACK(token::Enumerate);
DATAPACK(token::VariantBegin);
DATAPACK(token::VariantNext);
DATAPACK(token::BinaryData);
DATAPACK(token::ObjectBegin);
DATAPACK(token::ObjectEnd);
DATAPACK(token::ObjectNext);
DATAPACK(token::TupleBegin);
DATAPACK(token::TupleEnd);
DATAPACK(token::List);
DATAPACK_LABELLED_VARIANT(Token, 21);

DATAPACK_EMPTY(token::Optional);
DATAPACK_EMPTY(token::VariantEnd);
DATAPACK_EMPTY(token::TupleNext);

bool operator==(const Token& lhs, const Token& rhs);

} // namespace datpack
