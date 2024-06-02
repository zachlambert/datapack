#pragma once

#include "datapack/reader.hpp"
#include "datapack/object.hpp"


namespace datapack {

namespace btoken {

struct Optional {};
struct Enumerate {
    std::vector<std::string> labels;
    Enumerate(const std::vector<std::string>& labels): labels(labels) {}
    Enumerate(const std::vector<const char*>& labels) {
        for (const char* str: labels) {
            this->labels.push_back(std::string(str));
        }
    }
};
struct VariantBegin {
    std::vector<std::string> labels;
    VariantBegin(const std::vector<std::string>& labels): labels(labels) {}
    VariantBegin(const std::vector<const char*>& labels) {
        for (const char* str: labels) {
            this->labels.push_back(std::string(str));
        }
    }
};
struct VariantEnd {};
struct VariantNext {
    const std::string type;
    VariantNext(const std::string& type): type(type) {}
};

struct Binary {};

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

using BToken = std::variant<
    std::int32_t,
    std::int64_t,
    std::uint32_t,
    std::uint64_t,
    float,
    double,
    std::string,
    bool,
    btoken::Optional,
    btoken::Enumerate,
    btoken::VariantBegin,
    btoken::VariantEnd,
    btoken::VariantNext,
    btoken::Binary,
    btoken::ObjectBegin,
    btoken::ObjectEnd,
    btoken::ObjectNext,
    btoken::TupleBegin,
    btoken::TupleEnd,
    btoken::TupleNext,
    btoken::Map,
    btoken::List
>;

struct BinarySchema {
    std::vector<BToken> tokens;
};

class BinarySchemaBuilder: public Reader {
public:
    BinarySchemaBuilder(BinarySchema& schema):
        Reader(true, true, true),
        tokens(schema.tokens),
        first_element(false)
    {
        tokens.clear();
    }

    void value_i32(std::int32_t& value) {
        tokens.push_back(value);
    }

    void value_i64(std::int64_t& value) {
        tokens.push_back(value);
    }

    void value_u32(std::uint32_t& value) {
        tokens.push_back(value);
    }

    void value_u64(std::uint64_t& value) {
        tokens.push_back(value);
    }


    void value_f32(float& value) {
        tokens.push_back(value);
    }

    void value_f64(double& value) {
        tokens.push_back(value);
    }


    void value_string(std::string& value) {
        tokens.push_back(value);
    }

    void value_bool(bool& value) {
        tokens.push_back(value);
    }


    int enumerate(const std::vector<const char*>& labels) {
        tokens.push_back(btoken::Enumerate(labels));
        return 0;
    }

    bool optional() {
        tokens.push_back(btoken::Optional());
        return true;
    }

    void variant_begin(const std::vector<const char*>& labels) {
        tokens.push_back(btoken::VariantBegin(labels));
    }

    bool variant_match(const char* label) {
        tokens.push_back(btoken::VariantNext(label));
        return true;
    }

    void variant_end() {
        tokens.push_back(btoken::VariantEnd());
    }


    std::size_t binary_size() {
        tokens.push_back(btoken::Binary());
        return 0;
    }

    void binary_data(std::uint8_t* data) {
        // Do nothing
    }


    void object_begin() {
        tokens.push_back(btoken::ObjectBegin());
    }

    void object_end() {
        tokens.push_back(btoken::ObjectEnd());
    }

    void object_next(const char* key) {
        tokens.push_back(btoken::ObjectNext(key));
    }


    void tuple_begin() {
        tokens.push_back(btoken::TupleBegin());
    }

    void tuple_end() {
        tokens.push_back(btoken::TupleEnd());
    }

    void tuple_next() {
        tokens.push_back(btoken::TupleNext());
    }


    void map_begin() {
        tokens.push_back(btoken::Map());
        first_element = true;
    }

    void map_end() {
        // Do nothing
    }

    bool map_next(std::string& key) {
        if (first_element) {
            first_element = false;
            return true;
        }
        return false;
    }


    void list_begin() {
        tokens.push_back(btoken::List());
        first_element = true;
    }

    void list_end() {
        // Do nothing
    }

    bool list_next() {
        if (first_element) {
            first_element = false;
            return true;
        }
        return false;
    }


private:
    std::vector<BToken>& tokens;
    bool first_element;
};

template <readable T>
BinarySchema binary_schema() {
    T dummy;
    BinarySchema schema;
    BinarySchemaBuilder builder(schema);
    builder.value(dummy);
    return schema;
}

Object load_binary(const BinarySchema& schema, const std::vector<std::uint8_t>& data);

};
