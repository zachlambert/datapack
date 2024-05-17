#pragma once

#include <sstream>
#include <stack>
#include "datapack/writer.hpp"
#include "datapack/parser.hpp"

namespace datapack {

class JsonWriter: public Writer {
public:
    JsonWriter(std::ostream& os);

    void value_f64(const double& value) override;
    void value_i32(const int& value) override;

    void optional_begin(bool has_value) override;
    void optional_end() override;
    void variant_begin() override;
    void variant_match(const char* label) override;
    void variant_end() override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override;
    std::string result();

private:
    void assert_is_array(bool expected);
    void assert_at_value(bool expected);
    void indent();

    std::ostream& os;
    std::stack<bool> is_array;
    bool at_value;
    bool first_key_in_array;
};


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

} // namespace datpack
