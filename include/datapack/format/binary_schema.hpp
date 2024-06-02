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

struct BinaryData {};
struct BinaryBegin {
    const std::size_t stride;
    BinaryBegin(std::size_t stride):
        stride(stride)
    {}
};
struct BinaryEnd {};

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
    btoken::BinaryData,
    btoken::BinaryBegin,
    btoken::BinaryEnd,
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
        Reader(true, false, true),
        tokens(schema.tokens),
        first_element(false)
    {
        tokens.clear();
    }

    void value_i32(std::int32_t& value) override {
        tokens.push_back(value);
    }

    void value_i64(std::int64_t& value) override {
        tokens.push_back(value);
    }

    void value_u32(std::uint32_t& value) override {
        tokens.push_back(value);
    }

    void value_u64(std::uint64_t& value) override {
        tokens.push_back(value);
    }


    void value_f32(float& value) override {
        tokens.push_back(value);
    }

    void value_f64(double& value) override {
        tokens.push_back(value);
    }


    void value_string(std::string& value) override {
        tokens.push_back(value);
    }

    void value_bool(bool& value) override {
        tokens.push_back(value);
    }


    int enumerate(const std::vector<const char*>& labels) override {
        tokens.push_back(btoken::Enumerate(labels));
        return 0;
    }

    bool optional() override {
        tokens.push_back(btoken::Optional());
        return true;
    }

    void variant_begin(const std::vector<const char*>& labels) override {
        tokens.push_back(btoken::VariantBegin(labels));
    }

    bool variant_match(const char* label) override {
        tokens.push_back(btoken::VariantNext(label));
        return true;
    }

    void variant_end() override {
        tokens.push_back(btoken::VariantEnd());
    }


    std::tuple<const std::uint8_t*, std::size_t> binary_data() override {
        tokens.push_back(btoken::BinaryData());
        return std::make_tuple(nullptr, 0);
    }

    std::size_t binary_begin(std::size_t stride) override {
        tokens.push_back(btoken::BinaryBegin(stride));
        return stride; // stride * (fixed_size == 0 ? 1 : fixed_size);
    }

    void binary_end() override {
        tokens.push_back(btoken::BinaryEnd());
    }

    void object_begin() override {
        tokens.push_back(btoken::ObjectBegin());
    }

    void object_end() override {
        tokens.push_back(btoken::ObjectEnd());
    }

    void object_next(const char* key) override {
        tokens.push_back(btoken::ObjectNext(key));
    }


    void tuple_begin() override {
        tokens.push_back(btoken::TupleBegin());
    }

    void tuple_end() override {
        tokens.push_back(btoken::TupleEnd());
    }

    void tuple_next() override {
        tokens.push_back(btoken::TupleNext());
    }


    void map_begin() override {
        tokens.push_back(btoken::Map());
        first_element = true;
    }

    void map_end() override {
        // Do nothing
    }

    bool map_next(std::string& key) override {
        if (first_element) {
            first_element = false;
            return true;
        }
        return false;
    }


    void list_begin() override {
        tokens.push_back(btoken::List());
        first_element = true;
    }

    void list_end() override {
        // Do nothing
    }

    bool list_next() override {
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
BinarySchema create_binary_schema() {
    T dummy;
    BinarySchema schema;
    BinarySchemaBuilder builder(schema);
    builder.value(dummy);
    return schema;
}

Object load_binary(const BinarySchema& schema, const std::vector<std::uint8_t>& data);

};
