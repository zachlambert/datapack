#pragma once

#include <concepts>
#include <string>
#include <vector>
#include <variant>
#include "datapack/writer.hpp"
#include "datapack/constraint.hpp"


namespace datapack {

class Definer;

template <typename T>
concept defined = requires(Definer& definer, const T& value) {
    { define(definer, value) };
};

class Defined {
public:
    virtual void define(Definer&) const = 0;
};

inline void define(Definer& definer, const Defined& value) {
    value.define(definer);
}

template <typename T>
concept defined_binary = requires(Definer& definer, const T& value) {
    { define_binary(definer, value) };
};

namespace dtoken {

struct Optional {};
struct Enumerate {
    const std::vector<std::string> labels;
    Enumerate(const std::vector<std::string>& labels): labels(labels) {}
};
struct VariantBegin {};
struct VariantEnd {};
struct VariantNext {
    const std::string type;
    VariantNext(const std::string& type): type(type) {}
};

struct Binary {
    std::size_t expected_size;
    Binary(std::size_t expected_size): expected_size(expected_size) {}
};

struct ObjectBegin {};
struct ObjectEnd {};
struct ObjectNext {
    const std::string key;
    ObjectNext(const std::string& key): key(key) {}
};

struct TupleBegin {};
struct TupleEnd {};
struct TupleNext {};

struct Map {};

struct List {};

} // namespace dtoken

using DToken = std::variant<
    std::int32_t,
    std::int64_t,
    std::uint32_t,
    std::uint64_t,
    float,
    double,
    std::string,
    bool,
    dtoken::Optional,
    dtoken::Enumerate,
    dtoken::VariantBegin,
    dtoken::VariantEnd,
    dtoken::VariantNext,
    dtoken::Binary,
    dtoken::ObjectBegin,
    dtoken::ObjectEnd,
    dtoken::ObjectNext,
    dtoken::TupleBegin,
    dtoken::TupleEnd,
    dtoken::TupleNext,
    dtoken::Map,
    dtoken::List
>;

class Definer {
public:
    template <defined T>
    void value(const T& value) {
        define(*this, value);
    }

    template <defined T>
    void value(const char* key, const T& value) {
        object_next(key);
        define(*this, value);
    }

    template <defined T, is_constraint Constraint>
    void value(const T& value, const Constraint&) {
        define(*this, value);
    }

    template <defined T, is_constraint Constraint>
    void value(const char* key, const T& value, const Constraint&) {
        object_next(key);
        define(*this, value);
    }

    template <typename T>
    void value_primitive(T value) {
        tokens_.push_back(value);
    }
    void enumerate(const std::vector<const char*>& labels_cstr) {
        std::vector<std::string> labels;
        for (const auto& label: labels_cstr) {
            labels.push_back(label);
        }
        tokens_.push_back(dtoken::Enumerate(labels));
    }
    void optional() {
        tokens_.push_back(dtoken::Optional());
    }
    void variant_begin() {
        tokens_.push_back(dtoken::VariantBegin());
    }
    void variant_end() {
        tokens_.push_back(dtoken::VariantEnd());
    }
    void variant_next(const char* label) {
        tokens_.push_back(dtoken::VariantNext(std::string(label)));
    }

    void binary(std::size_t expected_size = 0) {
        tokens_.push_back(dtoken::Binary(expected_size));
    }

    template <defined_binary T>
    void value_binary(const T& value) {
        define_binary(*this, value);
    }

    template <defined_binary T>
    void value_binary(const char* key, const T& value) {
        object_next(key);
        define_binary(*this, value);
    }

    void object_begin() {
        tokens_.push_back(dtoken::ObjectBegin());
    }
    void object_end() {
        tokens_.push_back(dtoken::ObjectEnd());
    }
    void object_next(const char* key) {
        tokens_.push_back(dtoken::ObjectNext(key));
    }

    void tuple_begin() {
        tokens_.push_back(dtoken::TupleBegin());
    }
    void tuple_end() {
        tokens_.push_back(dtoken::TupleEnd());
    }
    void tuple_next() {
        tokens_.push_back(dtoken::TupleNext());
    }

    void map() {
        tokens_.push_back(dtoken::Map());
    }

    void list() {
        tokens_.push_back(dtoken::List());
    }

    const std::vector<DToken>& tokens() { return tokens_; }

private:
    std::vector<DToken> tokens_;
};

} // namespace datapack

std::ostream& operator<<(std::ostream& os, const std::vector<datapack::DToken>& tokens);
