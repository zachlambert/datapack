#pragma once

#include "datapack/token.hpp"
#include "datapack/object.hpp"

namespace datapack {

class Parser {
public:
    virtual std::optional<Token> next() = 0;
};

Object parse(Parser& parser);
Object parse(Parser&& parser);

} // namespace datapack
