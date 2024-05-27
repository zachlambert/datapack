#pragma once

#include <sstream>
#include <stack>
#include "datapack/writer.hpp"
#include "datapack/reader.hpp"
#include "datapack/object.hpp"
#include "datapack/util/object.hpp"

namespace datapack {

#if 0
class JsonWriter: public Writer {
public:
    JsonWriter(std::string& result);

    void value_i32(std::int32_t value) override;
    void value_i64(std::int64_t value) override;
    void value_u32(std::uint32_t value) override;
    void value_u64(std::uint64_t value) override;

    void value_f32(float value) override;
    void value_f64(double value) override;

    void value_string(const std::string&) override;
    void value_bool(bool value) override;

    void enumerate(int value, const std::vector<const char*>& labels) override;
    void optional(bool has_value) override;
    void variant_begin(const char* label, const std::vector<const char*>& labels) override;
    void variant_end() override;

    void binary(std::size_t size, const std::uint8_t* data) override;

    void object_begin() override { map_begin(); }
    void object_end() override { map_end(); }
    void object_next(const char* key) override { map_next(key); }

    void tuple_begin() override { list_begin(); }
    void tuple_end() override { list_end(); }
    void tuple_next() override { list_next(); }

    void map_begin() override;
    void map_end() override;
    void map_next(const std::string& key) override;

    void list_begin() override;
    void list_end() override;
    void list_next() override;

private:
    void indent();

    std::string& result;
    bool first_element;
    int depth;
};
#endif


Object load_json(const std::string& json);
std::string dump_json(ConstObject object);

template <readable T>
T read_json(const std::string& json) {
    Object object = load_json(json);
    T result;
    ObjectReader(object).value(result);
    return result;
}

template <writeable T>
std::string write_json(const T& value) {
    Object object;
    ObjectWriter(object).value(value);
    return dump_json(object);
}

#if 0
class JsonParser: public Parser {
public:
    JsonParser(const std::string& source);

    Token next() override;

private:
    void error(const std::string& error);

    static constexpr int EXPECT_ELEMENT = 1 << 0;
    static constexpr int EXPECT_VALUE = 1 << 1;
    static constexpr int EXPECT_END = 1 << 2;
    static constexpr int EXPECT_NEXT = 1 << 3;
    static constexpr int IS_OBJECT = 1 << 4;
    static constexpr int IS_ARRAY = 1 << 5;

    std::string_view source;
    std::size_t pos;
    std::stack<int> states;
};
#endif

} // namespace datpack
