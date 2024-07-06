#pragma once
#ifndef EMBEDDED

#include "datapack/datapack.hpp"
#include "datapack/labelled_variant.hpp"


namespace datapack {

namespace token {

struct Optional {};

struct Enumerate {
    std::vector<std::string> labels;
    Enumerate() {}
    Enumerate(const std::vector<std::string>& labels): labels(labels) {}
    Enumerate(const std::vector<const char*>& labels) {
        for (const char* str: labels) {
            this->labels.push_back(std::string(str));
        }
    }
};
struct VariantBegin {
    std::vector<std::string> labels;
    VariantBegin() {}
    VariantBegin(const std::vector<std::string>& labels): labels(labels) {}
    VariantBegin(const std::vector<const char*>& labels) {
        for (const char* str: labels) {
            this->labels.push_back(std::string(str));
        }
    }
};
struct VariantEnd {};
struct VariantNext {
    std::string type;
    VariantNext() {}
    VariantNext(const std::string& type): type(type) {}
};

struct BinaryData {};

struct ObjectBegin {};
struct ObjectEnd {};
struct ObjectNext {
    std::string key;
    ObjectNext() {}
    ObjectNext(const std::string& key): key(key) {}
};

struct TupleBegin {};
struct TupleEnd {};
struct TupleNext {};

struct Map {};

struct List {
    bool is_array;
    List(bool is_array = false):
        is_array(is_array)
    {}
};

} // namespace dtoken

using Token = std::variant<
    std::int32_t,
    std::int64_t,
    std::uint32_t,
    std::uint64_t,
    float,
    double,
    std::string,
    bool,
    token::Optional,
    token::Enumerate,
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
    token::Map,
    token::List
>;

DATAPACK(token::Enumerate)
DATAPACK(token::VariantBegin)
DATAPACK(token::VariantNext)
DATAPACK(token::ObjectNext)
DATAPACK(token::List)
DATAPACK_LABELLED_VARIANT(Token)

DATAPACK_EMPTY(token::Optional)
DATAPACK_EMPTY(token::VariantEnd)
DATAPACK_EMPTY(token::BinaryData)
DATAPACK_EMPTY(token::ObjectBegin)
DATAPACK_EMPTY(token::ObjectEnd)
DATAPACK_EMPTY(token::TupleBegin)
DATAPACK_EMPTY(token::TupleEnd)
DATAPACK_EMPTY(token::TupleNext)
DATAPACK_EMPTY(token::Map)

bool operator==(const Token& lhs, const Token& rhs);

} // namespace datpack
#endif
