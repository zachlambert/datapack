#pragma once

#include "datapack/token.hpp"
#include "datapack/object.hpp"

namespace datapack {

class ParseException: public std::exception {
public:
    ParseException(const std::string& message):
        message(message)
    {}

private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

class Parser {
public:
    virtual std::optional<Token> next() = 0;
};

Object parse(Parser& parser);
Object parse(Parser&& parser);

class ParserReader: public Reader {
public:
    ParserReader(Parser& parser);

    int i32() override;
    long i64() override;
    float f32() override;
    double f64() override;
    std::string string() override;
    bool boolean() override;
    bool null() override;
    binary_t binary() override;

    void object_begin() override;
    void object_end() override;
    void object_element(const std::string& key) override;

    void array_begin() override;
    void array_end() override;
    bool array_element() override;

private:
    Parser& parser;
};

} // namespace datapack
