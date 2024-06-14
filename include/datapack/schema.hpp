#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include "datapack/object.hpp"
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

struct Schema {
    std::vector<Token> tokens;
};

class SchemaBuilder: public Reader {
public:
    SchemaBuilder(Schema& schema):
        Reader(false, false, true),
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
        tokens.push_back(token::Enumerate(labels));
        return 0;
    }

    bool optional() override {
        tokens.push_back(token::Optional());
        return true;
    }

    void variant_begin(const std::vector<const char*>& labels) override {
        tokens.push_back(token::VariantBegin(labels));
    }

    bool variant_match(const char* label) override {
        tokens.push_back(token::VariantNext(label));
        return true;
    }

    void variant_end() override {
        tokens.push_back(token::VariantEnd());
    }


    std::tuple<const std::uint8_t*, std::size_t> binary_data() override {
        tokens.push_back(token::BinaryData());
        return std::make_tuple(nullptr, 0);
    }

    void object_begin() override {
        tokens.push_back(token::ObjectBegin());
    }

    void object_end() override {
        tokens.push_back(token::ObjectEnd());
    }

    void object_next(const char* key) override {
        tokens.push_back(token::ObjectNext(key));
    }


    void tuple_begin() override {
        tokens.push_back(token::TupleBegin());
    }

    void tuple_end() override {
        tokens.push_back(token::TupleEnd());
    }

    void tuple_next() override {
        tokens.push_back(token::TupleNext());
    }


    void map_begin() override {
        tokens.push_back(token::Map());
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


    void list_begin(bool is_array) override {
        tokens.push_back(token::List(is_array));
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
    std::vector<Token>& tokens;
    bool first_element;
};

template <readable T>
Schema create_schema() {
    T dummy;
    Schema schema;
    SchemaBuilder builder(schema);
    builder.value(dummy);
    return schema;
}

void use_schema(const Schema& schema, Reader& reader, Writer& writer);
inline void use_schema(const Schema& schema, Reader&& reader, Writer&& writer) {
    use_schema(schema, reader, writer);
}

bool operator==(const Schema& lhs, const Schema& rhs);

DATAPACK(token::Enumerate)
DATAPACK(token::VariantBegin)
DATAPACK(token::VariantNext)
DATAPACK(token::ObjectNext)
DATAPACK(token::List)
DATAPACK_LABELLED_VARIANT(Token)
DATAPACK(Schema)

DATAPACK_EMPTY(token::Optional)
DATAPACK_EMPTY(token::VariantEnd)
DATAPACK_EMPTY(token::BinaryData)
DATAPACK_EMPTY(token::ObjectBegin)
DATAPACK_EMPTY(token::ObjectEnd)
DATAPACK_EMPTY(token::TupleBegin)
DATAPACK_EMPTY(token::TupleEnd)
DATAPACK_EMPTY(token::TupleNext)
DATAPACK_EMPTY(token::Map)

} // namespace datpack
