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

} // namespace datapack
