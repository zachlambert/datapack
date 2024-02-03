#pragma once

#include <sstream>
#include <stack>
#include "datapack/writer.hpp"
#include "datapack/parser.hpp"

namespace datapack {

class JsonWriter: public Writer {
public:
    JsonWriter();

    void i32(int value) override;
    void i64(long value) override;
    void f32(float value) override;
    void f64(double value) override;
    void string(const std::string& value) override;
    void boolean(bool value) override;
    void null() override;
    void binary(const binary_t& value) override;

    void object_begin() override;
    void object_end() override;
    void object_element(const std::string& key) override;

    void array_begin() override;
    void array_end() override;
    void array_element() override;

    std::string result();

private:
    void assert_is_array(bool expected);
    void assert_at_value(bool expected);
    void indent();

    std::stringstream ss;
    std::stack<bool> is_array;
    bool at_value;
    bool first_key_in_array;
};

class JsonParser: public Parser {
public:
    JsonParser(const std::string& source);

    std::optional<Token> next() override;

private:
    static constexpr int EXPECT_KEY = 1 << 0;
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
